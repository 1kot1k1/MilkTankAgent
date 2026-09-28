#include "HttpApiServer.h"
#include "AppController.h"
#include "SessionManager.h"
#include "Logger.h"

#include "HttpLib.h"
#include "json.hpp"

#include <deque>
#include <fstream>

using json = nlohmann::json;

HttpApiServer::HttpApiServer(
    AppController& controller,
    SessionManager& sessions)
    : m_controller(controller)
    , m_sessions(sessions)
{
}

HttpApiServer::~HttpApiServer()
{
    stop();

    delete m_server;
}

void HttpApiServer::start(
    unsigned short port,
    const std::string& wwwRoot)
{
    m_server = new httplib::Server();

    // Frontend static files (HTML/CSS/JS from Track D). Note: this
    // does NOT require a token — login.html itself must be reachable
    // by anyone, otherwise nobody could ever log in.
    m_server->set_mount_point("/", wwwRoot);

    // cpp-httplib doesn't automatically serve index.html for the
    // bare "/" address — send visitors to the login page, which
    // itself forwards to the dashboard once they're authenticated.
    m_server->Get(
        "/",
        [](const httplib::Request&, httplib::Response& res)
        {
            res.set_redirect("/login.html");
        });

    // Login: { "password": "..." } -> { "ok": true, "token": "..." }
    m_server->Post(
        "/api/login",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            std::string password;

            try
            {
                const json body = json::parse(req.body);
                password = body.value("password", "");
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"ok":false,"error":"bad request"})",
                    "application/json");
                return;
            }

            if (!m_controller.verifyPassword(password))
            {
                res.status = 401;
                res.set_content(
                    R"({"ok":false})",
                    "application/json");
                return;
            }

            const std::string token = m_sessions.issueToken();

            json response;
            response["ok"] = true;
            response["token"] = token;

            res.set_content(response.dump(), "application/json");
        });

    // Current snapshot of all devices, used when the page first loads
    // (before any WebSocket event has arrived yet). Requires a valid
    // session token from /api/login.
    m_server->Get(
        "/api/devices",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            const std::string token = req.get_param_value("token");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            json response = json::array();

            for (const Device& device : m_controller.devices())
            {
                json item;

                item["uuid"] = device.uuid;
                item["name"] = device.name;
                item["enabled"] = device.enabled;
                item["weight"] = device.lastWeight;
                item["stable"] = device.stable;
                item["overload"] = device.overload;
                item["state"] = static_cast<int>(device.state);
                item["online"] = (device.state == DeviceState::Online);
                item["port"] = device.port;
                item["baudRate"] = device.baudRate;
                item["address"] = device.address;

                response.push_back(item);
            }

            res.set_content(response.dump(), "application/json");
        });

    // Add a new device.
    // Body: { "token":"...", "name":"...", "port":"/dev/ttyUSB0",
    //         "baudRate":115200, "address":1, "enabled":true }
    m_server->Post(
        "/api/devices",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            json body;

            try
            {
                body = json::parse(req.body);
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"bad request"})",
                    "application/json");
                return;
            }

            const std::string token = body.value("token", "");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            const std::string name = body.value("name", "");
            const std::string port = body.value("port", "");

            if (name.empty() || port.empty())
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"name and port are required"})",
                    "application/json");
                return;
            }

            Device device;
            device.name = name;
            device.port = port;
            device.baudRate = body.value("baudRate", 115200);
            device.address = body.value("address", 1);
            device.enabled = body.value("enabled", true);
            device.uuid = m_controller.generateDeviceUuid();

            if (!m_controller.addDevice(device))
            {
                res.status = 500;
                res.set_content(
                    R"({"error":"failed to save device, check server log"})",
                    "application/json");
                return;
            }

            json response;
            response["ok"] = true;
            response["uuid"] = device.uuid;

            res.set_content(response.dump(), "application/json");
        });

    // Edit an existing device's parameters.
    // PUT /api/devices/<uuid>
    // Body: { "token":"...", "name":"...", "port":"...",
    //         "baudRate":115200, "address":1 }
    m_server->Put(
        R"(/api/devices/([^/]+))",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            json body;

            try
            {
                body = json::parse(req.body);
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"bad request"})",
                    "application/json");
                return;
            }

            const std::string token = body.value("token", "");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            const std::string uuid = req.matches[1];

            Device device;

            if (!m_controller.getDevice(uuid, device))
            {
                res.status = 404;
                res.set_content(
                    R"({"error":"device not found"})",
                    "application/json");
                return;
            }

            const std::string name = body.value("name", "");
            const std::string port = body.value("port", "");

            if (name.empty() || port.empty())
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"name and port are required"})",
                    "application/json");
                return;
            }

            device.name = name;
            device.port = port;
            device.baudRate = body.value("baudRate", device.baudRate);
            device.address = body.value("address", device.address);

            if (!m_controller.updateDevice(device))
            {
                res.status = 500;
                res.set_content(
                    R"({"error":"failed to update device, check server log"})",
                    "application/json");
                return;
            }

            res.set_content(R"({"ok":true})", "application/json");
        });

    // Remove a device by uuid: DELETE /api/devices/<uuid>?token=...
    m_server->Delete(
        R"(/api/devices/([^/]+))",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            const std::string token = req.get_param_value("token");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            const std::string uuid = req.matches[1];

            if (!m_controller.removeDevice(uuid))
            {
                res.status = 404;
                res.set_content(
                    R"({"error":"device not found"})",
                    "application/json");
                return;
            }

            res.set_content(R"({"ok":true})", "application/json");
        });

    // Enable/disable a device: POST /api/devices/<uuid>/enabled
    // Body: { "token":"...", "enabled": true }
    m_server->Post(
        R"(/api/devices/([^/]+)/enabled)",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            json body;

            try
            {
                body = json::parse(req.body);
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"bad request"})",
                    "application/json");
                return;
            }

            const std::string token = body.value("token", "");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            const std::string uuid = req.matches[1];
            const bool enabled = body.value("enabled", true);

            if (!m_controller.setDeviceEnabled(uuid, enabled))
            {
                res.status = 404;
                res.set_content(
                    R"({"error":"device not found"})",
                    "application/json");
                return;
            }

            res.set_content(R"({"ok":true})", "application/json");
        });

    // Read current application settings.
    // GET /api/settings?token=...
    m_server->Get(
        "/api/settings",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            const std::string token = req.get_param_value("token");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            const AppSettings settings = m_controller.settings();

            json response;
            response["serverHost"] = settings.serverHost;
            response["apiPath"] = settings.apiPath;
            response["companyUuid"] = settings.companyUuid;
            response["farmUuid"] = settings.farmUuid;
            response["herdUuid"] = settings.herdUuid;
            response["pollInterval"] = settings.pollInterval;
            response["sendInterval"] = settings.sendInterval;
            response["retentionDays"] = settings.retentionDays;
            response["verifyTlsCertificate"] = settings.verifyTlsCertificate;

            res.set_content(response.dump(), "application/json");
        });

    // Update application settings.
    // Body: { "token":"...", "serverHost":"...", "apiPath":"...",
    //         "companyUuid":"...", "farmUuid":"...", "herdUuid":"...",
    //         "pollInterval":5, "sendInterval":30, "retentionDays":30,
    //         "verifyTlsCertificate":false }
    m_server->Post(
        "/api/settings",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            json body;

            try
            {
                body = json::parse(req.body);
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"bad request"})",
                    "application/json");
                return;
            }

            const std::string token = body.value("token", "");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            AppSettings settings;
            settings.serverHost = body.value("serverHost", "");
            settings.apiPath = body.value("apiPath", "");
            settings.companyUuid = body.value("companyUuid", "");
            settings.farmUuid = body.value("farmUuid", "");
            settings.herdUuid = body.value("herdUuid", "");
            settings.pollInterval = body.value("pollInterval", 5);
            settings.sendInterval = body.value("sendInterval", 30);
            settings.retentionDays = body.value("retentionDays", 30);
            settings.verifyTlsCertificate =
                body.value("verifyTlsCertificate", false);

            if (!m_controller.updateSettings(settings))
            {
                res.status = 500;
                res.set_content(
                    R"({"error":"failed to save settings, check server log"})",
                    "application/json");
                return;
            }

            res.set_content(R"({"ok":true})", "application/json");
        });

    // Change the admin password.
    // Body: { "token":"...", "oldPassword":"...", "newPassword":"..." }
    m_server->Post(
        "/api/change-password",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            json body;

            try
            {
                body = json::parse(req.body);
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"bad request"})",
                    "application/json");
                return;
            }

            const std::string token = body.value("token", "");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            const std::string oldPassword = body.value("oldPassword", "");
            const std::string newPassword = body.value("newPassword", "");

            if (newPassword.empty())
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"new password is required"})",
                    "application/json");
                return;
            }

            if (!m_controller.changePassword(oldPassword, newPassword))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"current password is incorrect"})",
                    "application/json");
                return;
            }

            res.set_content(R"({"ok":true})", "application/json");
        });

    // Database size + manual purge (used by the settings page).
    m_server->Get(
        "/api/database-info",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            const std::string token = req.get_param_value("token");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            json response;
            response["sizeBytes"] = m_controller.databaseSizeBytes();

            res.set_content(response.dump(), "application/json");
        });

    m_server->Post(
        "/api/database/clear",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            json body;

            try
            {
                body = json::parse(req.body);
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"bad request"})",
                    "application/json");
                return;
            }

            const std::string token = body.value("token", "");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            if (!m_controller.clearDatabase())
            {
                res.status = 500;
                res.set_content(
                    R"({"error":"failed to clear database"})",
                    "application/json");
                return;
            }

            res.set_content(R"({"ok":true})", "application/json");
        });

    // Recent log lines, for the journal page's initial load
    // (live updates after that arrive over the WebSocket).
    // GET /api/logs?token=...&lines=200
    m_server->Get(
        "/api/logs",
        [this](const httplib::Request& req, httplib::Response& res)
        {
            const std::string token = req.get_param_value("token");

            if (!m_sessions.isValid(token))
            {
                res.status = 401;
                res.set_content(
                    R"({"error":"unauthorized"})",
                    "application/json");
                return;
            }

            int maxLines = 200;

            if (req.has_param("lines"))
            {
                try
                {
                    maxLines = std::stoi(req.get_param_value("lines"));
                }
                catch (const std::exception&)
                {
                    maxLines = 200;
                }
            }

            std::ifstream file("logs/MilkTankAgent.log");

            std::deque<std::string> lines;
            std::string line;

            while (std::getline(file, line))
            {
                lines.push_back(line);

                if (static_cast<int>(lines.size()) > maxLines)
                {
                    lines.pop_front();
                }
            }

            json response = json::array();

            for (const auto& l : lines)
            {
                response.push_back(l);
            }

            res.set_content(response.dump(), "application/json");
        });

    m_thread = std::thread(
        [this, port]()
        {
            Logger::info(
                "HTTP API server listening on port "
                + std::to_string(port));

            if (!m_server->listen("0.0.0.0", port))
            {
                Logger::error(
                    "HTTP API server failed to start on port "
                    + std::to_string(port));
            }
        });
}

void HttpApiServer::stop()
{
    if (m_server != nullptr)
    {
        m_server->stop();
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}
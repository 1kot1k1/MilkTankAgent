#include "WebSocketBroadcaster.h"
#include "SessionManager.h"
#include "Logger.h"

namespace
{
    // Extracts "token" from a query string like "token=abc123&x=1"
    std::string extractToken(const std::string& query)
    {
        const std::string key = "token=";

        const std::size_t start = query.find(key);

        if (start == std::string::npos)
        {
            return "";
        }

        const std::size_t valueStart = start + key.size();
        const std::size_t end = query.find('&', valueStart);

        if (end == std::string::npos)
        {
            return query.substr(valueStart);
        }

        return query.substr(valueStart, end - valueStart);
    }
}

WebSocketBroadcaster::WebSocketBroadcaster(SessionManager& sessions)
    : m_sessions(sessions)
{
}

bool WebSocketBroadcaster::onValidate(ConnectionHandle handle)
{
    const Server::connection_ptr connection =
        m_server.get_con_from_hdl(handle);

    const std::string query =
        connection->get_uri()->get_query();

    const std::string token = extractToken(query);

    if (!m_sessions.isValid(token))
    {
        Logger::warning(
            "WebSocket connection rejected: invalid or missing token.");

        return false;
    }

    return true;
}

void WebSocketBroadcaster::onOpen(ConnectionHandle handle)
{
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_connections.insert(handle);

    Logger::info(
        "Web client connected. Total: "
        + std::to_string(m_connections.size()));
}

void WebSocketBroadcaster::onClose(ConnectionHandle handle)
{
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_connections.erase(handle);

    Logger::info(
        "Web client disconnected. Total: "
        + std::to_string(m_connections.size()));
}

void WebSocketBroadcaster::start(unsigned short port)
{
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.clear_error_channels(websocketpp::log::elevel::all);

    m_server.init_asio();

    m_server.set_reuse_addr(true);

    m_server.set_validate_handler(
        [this](ConnectionHandle handle)
        {
            return onValidate(handle);
        });

    m_server.set_open_handler(
        [this](ConnectionHandle handle)
        {
            onOpen(handle);
        });

    m_server.set_close_handler(
        [this](ConnectionHandle handle)
        {
            onClose(handle);
        });

    m_server.listen(port);
    m_server.start_accept();

    m_thread = std::thread(
        [this]()
        {
            try
            {
                m_server.run();
            }
            catch (const std::exception& exception)
            {
                Logger::error(
                    std::string("WebSocket server crashed: ")
                    + exception.what());
            }
        });

    Logger::info(
        "WebSocket server listening on port "
        + std::to_string(port));
}

void WebSocketBroadcaster::stop()
{
    m_server.stop_listening();

    {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);

        for (const auto& handle : m_connections)
        {
            websocketpp::lib::error_code errorCode;

            m_server.close(
                handle,
                websocketpp::close::status::going_away,
                "Server shutting down",
                errorCode);
        }
    }

    m_server.stop();

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void WebSocketBroadcaster::broadcast(const std::string& json)
{
    // post() marshals this onto the server's own thread, so the
    // calling thread (e.g. the device polling loop) never waits on
    // network I/O.
    m_server.get_io_service().post(
        [this, json]()
        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);

            for (const auto& handle : m_connections)
            {
                websocketpp::lib::error_code errorCode;

                m_server.send(
                    handle,
                    json,
                    websocketpp::frame::opcode::text,
                    errorCode);

                if (errorCode)
                {
                    Logger::warning(
                        "Failed to send to a web client: "
                        + errorCode.message());
                }
            }
        });
}
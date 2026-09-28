#pragma once

#include <string>
#include <thread>

class AppController;
class SessionManager;

namespace httplib
{
    class Server;
}

class HttpApiServer
{
public:
    HttpApiServer(
        AppController& controller,
        SessionManager& sessions);

    ~HttpApiServer();

    // wwwRoot — folder with the frontend files (HTML/CSS/JS) to serve
    void start(unsigned short port, const std::string& wwwRoot);
    void stop();

private:
    AppController& m_controller;
    SessionManager& m_sessions;
    httplib::Server* m_server = nullptr;
    std::thread m_thread;
};
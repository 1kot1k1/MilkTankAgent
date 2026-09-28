#pragma once

#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class SessionManager;

class WebSocketBroadcaster
{
public:
    using Server = websocketpp::server<websocketpp::config::asio>;
    using ConnectionHandle = websocketpp::connection_hdl;

    explicit WebSocketBroadcaster(SessionManager& sessions);

    void start(unsigned short port);
    void stop();

    // Thread-safe: can be called from any thread (e.g. the device
    // polling thread via an EventBus subscriber). Internally posts
    // the actual send work onto the WebSocket server's own thread,
    // so the caller never blocks on network I/O.
    void broadcast(const std::string& json);

private:
    SessionManager& m_sessions;

    Server m_server;
    std::thread m_thread;

    std::mutex m_connectionsMutex;
    std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> m_connections;

    // Runs before a connection is accepted. Returning false rejects
    // the handshake — used to require ?token=... in the WS URL.
    bool onValidate(ConnectionHandle handle);

    void onOpen(ConnectionHandle handle);
    void onClose(ConnectionHandle handle);
};
#include "WebSocketBroadcaster.h"
#include "SessionManager.h"

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    SessionManager sessions;

    // Issue a token by hand, since there's no login server in this
    // isolated test — normally the browser gets this from /api/login.
    const std::string token = sessions.issueToken();

    std::cout << "Test token: " << token << std::endl;
    std::cout << "Edit test_websocket.html and set the WebSocket URL to:" << std::endl;
    std::cout << "  ws://<pi-ip>:8081/?token=" << token << std::endl;

    WebSocketBroadcaster broadcaster(sessions);

    broadcaster.start(8081);

    std::cout << "WebSocket server running on port 8081." << std::endl;
    std::cout << "Sending a fake message every 2 seconds. Ctrl+C to stop." << std::endl;

    double weight = 0.0;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        weight += 1.5;

        const std::string json =
            R"({"uuid":"test","weight":)" +
            std::to_string(weight) +
            R"(,"stable":true})";

        std::cout << "Broadcasting: " << json << std::endl;

        broadcaster.broadcast(json);
    }

    return 0;
}
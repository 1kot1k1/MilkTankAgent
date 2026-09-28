#include "AppController.h"
#include "SessionManager.h"
#include "HttpApiServer.h"
#include "WebSocketBroadcaster.h"
#include "Logger.h"

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    AppController controller;

    if (!controller.initializeStorage())
    {
        std::cout << "initializeStorage() FAILED" << std::endl;
        return 1;
    }

    SessionManager sessions;

    HttpApiServer httpApi(controller, sessions);
    httpApi.start(8080, "./www");

    WebSocketBroadcaster wsBroadcaster(sessions);
    wsBroadcaster.start(8081);

    // Bridge: whenever a device event happens (from the polling loop,
    // in the real app), push it to connected browsers. Here we fake
    // it with a timer, since we don't have real devices wired in this
    // test — same idea as the future real EventBus subscriber.
    std::cout << "Full stack running." << std::endl;
    std::cout << "Open http://<pi-ip>:8080/login.html in a browser." << std::endl;
    std::cout << "Default password: 1234" << std::endl;
    std::cout << "Sending a fake device update every 2 seconds so you can watch the dashboard move." << std::endl;

    double weight = 0.0;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        weight += 1.5;

        if (weight > 200.0)
        {
            weight = 0.0;
        }

        const std::string json =
            R"({"uuid":"test-device-1","name":"Тестовый танк","weight":)" +
            std::to_string(weight) +
            R"(,"stable":true,"overload":false,"online":true})";

        wsBroadcaster.broadcast(json);
    }

    return 0;
}
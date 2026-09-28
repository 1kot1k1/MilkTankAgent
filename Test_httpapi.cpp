#include "HttpApiServer.h"
#include "AppController.h"
#include "SessionManager.h"

#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    AppController controller;

    if (!controller.initializeStorage())
    {
        std::cout << "initializeStorage() FAILED" << std::endl;
        return 1;
    }

    std::cout << "initializeStorage() OK" << std::endl;

    SessionManager sessions;
    HttpApiServer api(controller, sessions);

    api.start(8080, "./www");

    std::cout << "HTTP API running on port 8080." << std::endl;
    std::cout << "Try: curl http://localhost:8080/api/devices  (should be 401, no token)" << std::endl;
    std::cout << "Try: curl -X POST http://localhost:8080/api/login -d '{\"password\":\"1234\"}'" << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    std::this_thread::sleep_for(std::chrono::hours(24));

    return 0;
}
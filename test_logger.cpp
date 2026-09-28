#include "Logger.h"

#include <iostream>

int main()
{
    Logger::info("test info message");
    Logger::warning("test warning message");
    Logger::error("test error message");

    std::cout << "OK" << std::endl;
    return 0;
}
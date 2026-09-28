#include "AppController.h"
#include "MainWindow.h"

#include <QApplication>

#include <thread>
#include <windows.h>

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE instanceMutex =
        CreateMutexW(
            nullptr,
            TRUE,
            L"MilkTankAgent.SingleInstance");

    if (instanceMutex == nullptr)
    {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxW(
            nullptr,
            L"MilkTankAgent \u0443\u0436\u0435 "
            L"\u0437\u0430\u043f\u0443\u0449\u0435\u043d.",
            L"MilkTankAgent",
            MB_OK | MB_ICONINFORMATION);

        CloseHandle(instanceMutex);

        return 0;
    }

    QApplication qtApplication(
        argc,
        argv);

    AppController controller;
    MainWindow window(controller);
    window.show();

    std::thread agentThread(
        [&controller]()
        {
            controller.run();
        });

    const int result =
        qtApplication.exec();

    controller.stop();

    if (agentThread.joinable())
    {
        agentThread.join();
    }
    ReleaseMutex(instanceMutex);
    CloseHandle(instanceMutex);
    return result;
}
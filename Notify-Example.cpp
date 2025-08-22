
#include <thread>
#include <iostream>
#include <chrono>

#include "SocketLib/SocketLib.h"

#if defined (_WIN32) || defined (_WIN64)
#include <conio.h>
#else   // Linux
#include <termios.h>
#include <time.h>
auto _kbhit = []() -> int
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
};
#define localtime_s(x,y) localtime_r(y,x)
#endif

int main(int argc, const char* argv[])
{
#if defined(_WIN32) || defined(_WIN64)
    // Detect Memory Leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

    BaseSocket::EnumIpAddresses([](int iAddrType, const std::string& strAddr, int iPort, void*) -> int
    {
        std::cout << "IP Address: " << strAddr << ", Port: " << iPort << ", Type: " << iAddrType << std::endl;
        return 0; // continue enumeration
    }, nullptr);

    BaseSocket::SetAddrNotifyCallback([](bool bAdded, const std::string& strAddr, int iPort, int iAddrType)
    {
        std::cout << "Address " << (bAdded ? "added" : "removed") << ": " << strAddr << ", Port: " << iPort << ", Type: " << iAddrType << std::endl;
    });

    const char caZeichen[] = "\\|/-";
    int iIndex{0};
    while (_kbhit() == 0)
    {
        std::cout << '\r' << caZeichen[iIndex++] << std::flush;
        if (iIndex > 3) iIndex = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    return 0;
}

/* Copyright (C) 2016-2019 Thomas Hauck - All Rights Reserved.

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT

   The author would be happy if changes and
   improvements were reported back to him.

   Author:  Thomas Hauck
   Email:   Thomas@fam-hauck.de
*/

#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "SocketLib/SocketLib.h"

using namespace std;

#if defined (_WIN32) || defined (_WIN64)
#include <conio.h>
#else   // Linux
#include <termios.h>
#include <time.h>
void _getch()
{
    struct termios t;
    tcgetattr(fileno(stdin), &t);
    t.c_lflag &= ~ICANON;
    tcsetattr(fileno(stdin), TCSANOW, &t);
    getchar();
    t.c_lflag |= ICANON;
    tcsetattr(fileno(stdin), TCSANOW, &t);
}
#define localtime_s(x,y) localtime_r(y,x)
#endif

void ServerThread(const bool* bStop)
{
    SslUdpSocket sock;
    bool bIsClosed = false;

    // 3 callback function to handle the server socket events
    sock.BindErrorFunction([&](BaseSocket*) { cout << "Server: socket error" << endl; });
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Server: socket closing" << endl; bIsClosed = true; });

    sock.BindFuncBytesReceived([&](UdpSocket* pUdpSocket)
    {
        const size_t nAvailable = pUdpSocket->GetBytesAvailable();

        auto spBuffer = make_unique<uint8_t[]>(nAvailable + 1);

        string strFrom;
        const size_t nRead = pUdpSocket->Read(&spBuffer[0], nAvailable, strFrom);

        if (nRead > 0)
        {
            string strRec(nRead, 0);
            copy(&spBuffer[0], &spBuffer[nRead], &strRec[0]);

            stringstream strOutput;
            const auto tNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
            struct tm stTime;
            if (localtime_s(&stTime, &tNow) == 0)
                strOutput << put_time(&stTime, "%a, %d %b %Y %H:%M:%S") << " - ";
            strOutput << strFrom.c_str() << " - Server received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

            cout << strOutput.str();

            strRec = "Server echo: " + strRec;
            sock.Write(&strRec[0], strRec.size(), strFrom);
        }
    });

    sock.AddCertificat("certs/server-cert.crt", "certs/server-key.pem");

    const bool bCreated = sock.CreateServerSide("0.0.0.0", 3461);

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    sock.Close();
    while (bIsClosed == false)
        this_thread::sleep_for(chrono::milliseconds(10));
}

void ClientThread(const bool* bStop)
{
    SslUdpSocket sock;
    bool bIsClosed = false;

    // 4 callback function to handle the client socket events
    sock.BindErrorFunction([&](BaseSocket*) { cout << "Client: socket error" << endl; });
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Client: socket closing" << endl; bIsClosed = true; });
    sock.BindFuncBytesReceived([&](UdpSocket* pUdpSocket)
    {
        const size_t nAvailable = pUdpSocket->GetBytesAvailable();

        auto spBuffer = make_unique<uint8_t[]>(nAvailable + 1);

        string strFrom;
        const size_t nRead = pUdpSocket->Read(spBuffer.get(), nAvailable, strFrom);

        if (nRead > 0)
        {
            string strRec(nRead, 0);
            copy(&spBuffer[0], &spBuffer[nRead], &strRec[0]);

            stringstream strOutput;
            const auto tNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
            struct tm stTime;
            if (localtime_s(&stTime, &tNow) == 0)
                strOutput << put_time(&stTime, "%a, %d %b %Y %H:%M:%S") << " - ";
            strOutput << strFrom.c_str() << " - Client received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

            cout << strOutput.str();
        }
    });

    sock.BindFuncSslInitDone([&](UdpSocket* pUdpSocket)
    {
        sock.Write("Hallo World", 11, "127.0.0.1:3461");
    });

    sock.AddCertificat("certs/client-cert.crt", "certs/client-key.pem");

    const bool bCreated = sock.CreateClientSide("0.0.0.0", 1911, "127.0.0.1:3461");

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    sock.Close();
    while (bIsClosed == false)
        this_thread::sleep_for(chrono::milliseconds(10));
}

int main(int argc, const char* argv[])
{
#if defined(_WIN32) || defined(_WIN64)
    // Detect Memory Leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

    bool bStop = false;
    thread thServer = thread(bind(ServerThread, &bStop));
    // we wait a second so the server is ready
    this_thread::sleep_for(chrono::milliseconds(1000));
    thread thClient = thread(bind(ClientThread, &bStop));

    _getch();

    bStop = true;

    thServer.join();
    thClient.join();

    return 0;
}

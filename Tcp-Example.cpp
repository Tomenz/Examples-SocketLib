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

#define WITHOUT_OPENSSL
#include "SocketLib/SocketLib.h"

using namespace std;

#if defined (_WIN32) || defined (_WIN64)
#include <conio.h>
#ifdef _DEBUG
#ifdef _WIN64
#pragma comment(lib, "x64/Debug/socketlib64d")
#else
#pragma comment(lib, "Debug/socketlib32d")
#endif
#else
#ifdef _WIN64
#pragma comment(lib, "x64/Release/socketlib64")
#else
#pragma comment(lib, "Release/socketlib32")
#endif
#endif
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#else   // Linux
#include <termios.h>
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
#endif

void ServerThread(bool* bStop)
{
    TcpServer sock;

    // 3 callback function to handle the server socket events
    sock.BindErrorFunction([&](BaseSocket*) { cout << "Server: socket error" << endl; });
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Server: socket closing" << endl; });

    sock.BindNewConnection([&](const vector<TcpSocket*>& lstSockets)
    {
        // a list with sockets how connected
        for (auto& pSocket : lstSockets)
        {
            if (pSocket != nullptr)
            {
                // 3 callback functions to handle the sockets events
                pSocket->BindFuncBytesReceived([&](TcpSocket* pSock)
                {
                    uint32_t nAvalible = pSock->GetBytesAvailible();

                    auto spBuffer = make_unique<unsigned char[]>(nAvalible + 1);

                    uint32_t nRead = pSock->Read(spBuffer.get(), nAvalible);

                    if (nRead > 0)
                    {
                        string strRec(nRead, 0);
                        copy(spBuffer.get(), spBuffer.get() + nRead, &strRec[0]);

                        stringstream strOutput;
                        strOutput << pSock->GetClientAddr() << " - Server received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

                        cout << strOutput.str();

                        strRec = "Server echo: " + strRec;
                        pSock->Write(&strRec[0], strRec.size());

                        pSock->Close();
                    }
                });
                pSocket->BindErrorFunction([&](BaseSocket* pSock)
                {
                    // there was an error, we close the socket
                    pSock->Close();
                    cout << "Server accept: socket error" << endl;
                });
                pSocket->BindCloseFunction([&](BaseSocket* pSock)
                {
                    // We let the socket destroy it self, use this on sockets received by the server
                    pSock->SelfDestroy();
                });

                pSocket->StartReceiving();  // start to receive data
            }
        }
    });


    // start der server socket
    bool bCreated = sock.Start("0.0.0.0", 3461);    // IPv6 use "::1" as address

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // Closing the server socket will not call the close callback
    sock.Close();
}

void ClientThread(bool* bStop)
{
    TcpSocket sock;
    bool bIsClosed = false;

    // client socket has 4 callback function
    sock.BindErrorFunction([&](BaseSocket*) { cout << "Client: socket error" << endl; });
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Client: socket closing" << endl; bIsClosed = true; });
    sock.BindFuncBytesReceived([&](TcpSocket* pTcpSocket)
    {
        uint32_t nAvalible = pTcpSocket->GetBytesAvailible();

        auto spBuffer = make_unique<unsigned char[]>(nAvalible + 1);

        uint32_t nRead = pTcpSocket->Read(spBuffer.get(), nAvalible);

        if (nRead > 0)
        {
            string strRec(nRead, 0);
            copy(spBuffer.get(), spBuffer.get() + nRead, &strRec[0]);

            stringstream strOutput;
            strOutput << pTcpSocket->GetClientAddr() << " - Client received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

            cout << strOutput.str();
        }
    });

    sock.BindFuncConEstablished([&](TcpSocket* pTcpSocket)
    {
        pTcpSocket->Write("Hallo World", 11);
    });

    bool bConnected = sock.Connect("127.0.0.1", 3461);
    if (bConnected == false)
        cout << "error creating client socket" << endl;

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // The Close call will call the Callback function above
    // but if we leave the thread, the Instance is destroyed
    // and we crash. So we disable the Callback by setting a null pointer
    if (bIsClosed == false)
    {
        sock.BindCloseFunction(nullptr);
        sock.Close();
    }
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


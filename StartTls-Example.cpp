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

void ServerThread(const bool* bStop)
{
    TcpServer sock;
    bool bClientConnected = false;

    // 3 callback function to handle the server socket events
    sock.BindErrorFunction([&](BaseSocket* pSock) { cout << "Server: socket error" << endl; pSock->Close(); }); // Must call Close function
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Server: socket closing" << endl; });

    sock.BindNewConnection([&](const vector<TcpSocket*>& lstSockets)
    {
        // a list with sockets new connected
        for (auto& pSocket : lstSockets)
        {
            if (pSocket != nullptr)
            {
                bClientConnected = true;

                // 3 callback functions to handle the sockets events
                pSocket->BindFuncBytesReceived([&](TcpSocket* pSock)
                {
                    const size_t nAvalible = pSock->GetBytesAvailible();

                    if (nAvalible == 0) // Socket closed on remote
                    {
                        pSock->Close();
                        return;
                    }

                    auto spBuffer = make_unique<uint8_t[]>(nAvalible + 1);

                    const size_t nRead = pSock->Read(spBuffer.get(), nAvalible);

                    if (nRead > 0)
                    {
                        string strRec(nRead, 0);
                        copy(&spBuffer[0], &spBuffer[nRead], &strRec[0]);

                        if (strRec == "STARTTLS")
                        {
                            SslTcpSocket* pSslTcpSocket = SslTcpSocket::SwitchToSll(pSock);
                            pSock->SelfDestroy();
                            if (pSslTcpSocket->AddServerCertificat("certs/ca-root.crt", "certs/127-0-0-1.crt", "certs/127-0-0-1-key.pem", nullptr) == false)
                            {
                                pSslTcpSocket->Close();
                                return;
                            }
                            pSslTcpSocket->SetAcceptState();
                            pSslTcpSocket->StartReceiving();
                            return;
                        }

                        stringstream strOutput;
                        strOutput << pSock->GetClientAddr() << " - Server received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

                        cout << strOutput.str();

                        strRec = "Server echo: " + strRec;
                        pSock->Write(&strRec[0], strRec.size());

                        //pSock->Close(); // Optional, if you don't close the socket, the connection stays open
                    }
                });
                pSocket->BindErrorFunction([&](BaseSocket* pSock)
                {
                    // there was an error, we close the socket
                    pSock->Close();
                    cout << "Server: accept socket error" << endl;
                });
                pSocket->BindCloseFunction([&](BaseSocket* pSock)
                {
                    // We let the socket destroy it self, use this on sockets received by the server
                    bClientConnected = false;
                    cout << "Server: accept socket closing" << endl;
                });

                pSocket->StartReceiving();  // start to receive data
            }
        }
    });


    // start der server socket
    const bool bCreated = sock.Start("0.0.0.0", 3461);    // IPv6 use "::1" as address

    while (*bStop == false || bClientConnected == true)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // Closing the server socket will not call the close callback
    sock.Close();
}

void ClientThread(const bool* bStop)
{
    auto sock = make_unique<TcpSocket>();
    bool bIsClosed = false;
    bool bFirstConnect = false;

    // client socket has 4 callback function
    sock->BindErrorFunction([&](BaseSocket* pSock) { cout << "Client: socket error" << endl; pSock->Close(); }); // Must call Close function
    sock->BindCloseFunction([&](BaseSocket*) { cout << "Client: socket closing" << endl; bIsClosed = true; });
    sock->BindFuncBytesReceived([&](TcpSocket* pTcpSocket)
    {
        const size_t nAvalible = pTcpSocket->GetBytesAvailible();

        if (nAvalible == 0) // Socket closed on remote
        {
            pTcpSocket->Close();
            return;
        }

        auto spBuffer = make_unique<unsigned char[]>(nAvalible + 1);

        const size_t nRead = pTcpSocket->Read(spBuffer.get(), nAvalible);

        if (nRead > 0)
        {
            string strRec(nRead, 0);
            copy(&spBuffer[0], &spBuffer[nRead], &strRec[0]);

            stringstream strOutput;
            strOutput << pTcpSocket->GetClientAddr() << " - Client received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

            cout << strOutput.str();
        }
    });

    sock->BindFuncConEstablished([&](TcpSocket* pTcpSocket)
    {
        if (bFirstConnect == false)
        {
            bFirstConnect = true;   // The Function will be called twice, after the TLS is established

            pTcpSocket->Write("STARTTLS", 8);
            // we wait until the STARTTLS message is send
            if(pTcpSocket->GetOutBytesInQue() > 0)
                this_thread::sleep_for(chrono::milliseconds(10));

            auto pSslTcpSocket = make_unique<SslTcpSocket>(pTcpSocket); // Make a new SslSocket
            pTcpSocket->SelfDestroy();  // The current TcpSocket should delete it self, after the Callback return
            // https://raw.githubusercontent.com/bagder/ca-bundle/master/ca-bundle.crt
            pSslTcpSocket->SetTrustedRootCertificates("certs/ca-root.crt");
            pSslTcpSocket->SetConnectState();
            pSslTcpSocket->StartReceiving();
            sock = move(pSslTcpSocket); // We use the new TlsSocket from here on
            return;
        }

        pTcpSocket->Write("Message in TLS connection", 25);
    });

    const bool bConnected = sock->Connect("127.0.0.1", 3461);
    if (bConnected == false)
        cout << "error creating client socket" << endl;

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    if (bIsClosed == false)
    {
        sock->Close();
        // We wait until the callback above was called
        while (bIsClosed == false)
            this_thread::sleep_for(chrono::milliseconds(10));
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
    // we wait a second that the server is ready
    this_thread::sleep_for(chrono::milliseconds(1000));
    thread thClient = thread(bind(ClientThread, &bStop));

    _getch();

    bStop = true;

    thServer.join();
    thClient.join();

    return 0;
}

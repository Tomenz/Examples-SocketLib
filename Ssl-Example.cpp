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

// see WinSock2.h or sys/socket.h
const int AF_UNSPEC = 0;    // IPv4 or IPv6
const int AF_INET   = 2;    // IPv4
#if defined (_WIN32) || defined (_WIN64)
const int AF_INET6  = 23;   // IPv6
#else
const int AF_INET6 = 10;   // IPv6
#endif

void ServerThread(const bool* bStop)
{
    SslTcpServer sock;

    // 3 callback function to handle the server socket events
    sock.BindErrorFunction([&](BaseSocket* pSock) { cout << "Server: server socket error" << endl << flush; pSock->Close(); }); // Must call Close function
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Server: server socket closing" << endl << flush; });

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
                    const size_t nAvalible = pSock->GetBytesAvailable();

                    if (nAvalible == 0) // Socket closed on remote
                    {
                        pSock->Close();
                        return;
                    }

                    auto spBuffer = make_unique<uint8_t[]>(nAvalible + 1);

                    const size_t nRead = pSock->Read(&spBuffer[0], nAvalible);

                    if (nRead > 0)
                    {
                        string strRec(nRead, 0);
                        copy(&spBuffer[0], &spBuffer[nRead], &strRec[0]);

                        stringstream strOutput;
                        strOutput << pSock->GetClientAddr() << " - Server received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

                        cout << strOutput.str() << flush;

                        strRec = "Server echo: " + strRec;
                        pSock->Write(&strRec[0], strRec.size());

                        //pSock->Close(); // Optional, if you don't close the socket, the connection stays open
                    }
                });
                pSocket->BindErrorFunction([&](BaseSocket* pSock)
                {
                    // there was an error, we close the socket
                    pSock->Close();
                    cout << "Server: accept socket error" << endl << flush;
                });
                pSocket->BindCloseFunction([&](BaseSocket* pSock)
                {
                    cout << "Server: accept socket closing" << endl << flush;
                });

                pSocket->StartReceiving();  // start to receive data

                pSocket->Write("Server Hello Message", 20);
            }
        }
    });


    sock.AddCertificat("certs/ca-root.crt", "certs/127-0-0-1.crt", "certs/127-0-0-1-key.pem");    // root certificate, host certificate, host key
    //sock.SetDHParameter("dh-file.txt");
    vector<string> Alpn({ { "h2" },{ "http/1.1" } });   // optional
    sock.SetAlpnProtokollNames(Alpn);

    // start der server socket
    const bool bCreated = sock.Start("0.0.0.0", 3461);    // IPv6 use "::1" as address

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // Closing the server socket will not call the close callback
    sock.Close();
}

void ClientThread(const bool* bStop)
{
    SslTcpSocket sock;
    const string strHost("localhost");
    bool bIsClosed = false;

    // client socket has 4 callback function
    sock.BindErrorFunction([&](BaseSocket* pSock) { cout << "Client: socket error" << endl << flush; pSock->Close(); }); // Must call Close function
    sock.BindCloseFunction([&](BaseSocket*) { cout << "Client: socket closing" << endl << flush; bIsClosed = true; });
    sock.BindFuncBytesReceived([&](TcpSocket* pTcpSocket)
    {
        size_t const nAvalible = pTcpSocket->GetBytesAvailable();

        if (nAvalible == 0) // Socket closed on remote
        {
            pTcpSocket->Close();
            return;
        }

        auto spBuffer = make_unique<uint8_t[]>(nAvalible + 1);

        const size_t nRead = pTcpSocket->Read(&spBuffer[0], nAvalible);

        if (nRead > 0)
        {
            string strRec(nRead, 0);
            copy(&spBuffer[0], &spBuffer[nRead], &strRec[0]);

            stringstream strOutput;
            strOutput << pTcpSocket->GetClientAddr() << " - Client received: " << nRead << " Bytes, \"" << strRec << "\"" << endl;

            cout << strOutput.str() <<  flush;
        }
    });

    sock.BindFuncConEstablished([&](TcpSocket* pTcpSocket)
    {
        const long nResult = dynamic_cast<SslTcpSocket*>(pTcpSocket)->CheckServerCertificate(strHost.c_str());
        if (nResult != 0)
            cout << "Connected Zertifikat not verified\r\n";

        // From Server selected Alpn Protocol
        string Protocoll = dynamic_cast<SslTcpSocket*>(pTcpSocket)->GetSelAlpnProtocol();

        pTcpSocket->Write("Hallo World", 11);
    });

    // Bundle of root certificates can be downloaded from here
    // https://curl.se/docs/caextract.html
    // https://raw.githubusercontent.com/bagder/ca-bundle/master/ca-bundle.crt
    sock.SetTrustedRootCertificates("certs/ca-root.crt");
    vector<string> Alpn({ { "h2" }, { "http/1.1" } });
    sock.SetAlpnProtokollNames(Alpn);

    const bool bConnected = sock.Connect(strHost.c_str(), 3461, AF_INET); // force to use IPv4. if above ::1 is used on the server, use here AF_INET6
    if (bConnected == false)
        cout << "error creating client socket" << endl << flush;

    while (*bStop == false)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // The Close call will call the Callback function above
    // but if we leave the thread, the Instance is destroyed
    // and we crash. So we disable the Callback by setting a null pointer
    if (bIsClosed == false)
    {
        //sock.BindCloseFunction(static_cast<function<void(BaseSocket*)>>(nullptr));
        sock.Close();
        // Alternativ we wait until the callback above was called
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
    // we wait a second so the server is ready
    this_thread::sleep_for(chrono::milliseconds(1000));
    thread thClient = thread(bind(ClientThread, &bStop));

    _getch();

    bStop = true;

    thServer.join();
    thClient.join();

    return 0;
}

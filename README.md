[![Codacy Badge](https://api.codacy.com/project/badge/Grade/f63ea9a3da7249218f1616032738ca77)](https://app.codacy.com/manual/Tomenz/Examples-SocketLib?utm_source=github.com&utm_medium=referral&utm_content=Tomenz/Examples-SocketLib&utm_campaign=Badge_Grade_Dashboard)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/Tomenz/Examples-SocketLib.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/Tomenz/Examples-SocketLib/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/Tomenz/Examples-SocketLib.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/Tomenz/Examples-SocketLib/context:cpp)

# Examples-SocketLib
Some examples for the use of the SocketLib. Can be compiled with VS2019 or Linux with g++ (gcc version > 6)

IMPOTEND: You must have the openssl (version 1.1.1) somewhere installed. You need to setup the include + library path in the visual studio project files. On linux you have to install the libssl-dev in order to compile the examples.

- The Ssl-Example is a classic Client-Server example, using TLS with openssl

- The Tcp-Example is the same as the Ssl-Example but without ssl encryption 

- The Dtls-Example is a UDP socket wit ssl encryption 

This is my first github project. If you have trouble to compile or run the samples, please let me know!

# Examples-SocketLib
Some examples for the use of the SocketLib. Can be compiled with VS2019 or Linux with g++ (gcc version > 6)

IMPOTEND: You have to have the openssl (version 1.1.1) somewhere installed. You need to setup the include + library path in the visual studio project files. On linux you have to install the libssl-dev in order to compile the examples.

- The Ssl-Example is a classic Client-Server example, using TLS with openssl

- The Tcp-Example is the same as the Ssl-Example but without ssl encryption 

- The Dtls-Example is a UDP socket wit ssl encryption 

This is my first github project. If you have trouble to compile or run the samples, please let me know!

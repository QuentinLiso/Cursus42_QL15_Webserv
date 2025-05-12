#include "MySocket.hpp"
#include "MyServer.hpp"
#include <exception>
#include <iostream>

// Test for MySocket :
// int main()
// {
//     try
//     {
//         MySocket    server;
//         server.setSocket("8080", 10);
//         std::cout << "Server is listening on port 8080...\n";

//         while (1)
//         {
//             int clientFd = server.acceptConnections();
//             send(clientFd, "Hello, client!\n", 15, 0);
//             close (clientFd);
//         }
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }
//     return (0);
// }


// Test for MyServer :

int main()
{
    try
    {
        std::vector<std::string> ports;
        ports.push_back("8080");
        MyServer    server(ports);
        server.runServer();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
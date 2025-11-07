#include "../include/Manager.hpp"

#include <iostream>
#include <csignal>

volatile bool sigintCatch = false;

int main(void)
{
    glnet::Manager& client = glnet::Manager::getInstance();

    // Initialize the manager as a client
    client.initialize(glnet::connection::Side::CLIENT);

    // Set the server endpoint to localhost:8080
    client.setServerEndpoint({LOCALHOST, 8080});

    // Create TCP and UDP connections
    client.createConnection(glnet::connection::Type::TCP);
    client.createConnection(glnet::connection::Type::UDP);

    std::signal(SIGINT, [](int signal) {
        if (signal == SIGINT) {
            sigintCatch = true;
        }
    });

    // Set up callbacks for both connections
    client.callbacks().setOnConnection([&](std::uint32_t clientId) {
        // Send a message to the server as plain text
        client.sendMessage(glnet::connection::Type::TCP, "Hello!");

        // Alternatively, send a message to the server as a byte vector
        client.sendMessage(glnet::connection::Type::TCP, {0, 0, 0, 0, 6, 'H', 'e', 'l', 'l', 'o', '!'});

        std::cout << "Client connected to server. "<< std::endl;
    });

    client.connectToServer();

    while (!sigintCatch) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}

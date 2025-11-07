
#include "../include/Manager.hpp"

#include <iostream>
#include <csignal>

#include <span>

volatile bool sigintCatch = false;

int main(void)
{
    glnet::Manager& server = glnet::Manager::getInstance();

    // Initialize the manager as a server
    server.initialize(glnet::connection::Side::SERVER);

    // Create TCP and UDP connections on localhost:8080
    server.createConnection(glnet::connection::Type::TCP, {LOCALHOST, 8080});
    server.createConnection(glnet::connection::Type::UDP, {LOCALHOST, 8080});

    std::signal(SIGINT, [](int signal) {
        if (signal == SIGINT) {
            sigintCatch = true;
        }
    });

    // Set up callbacks for both connections
    server.callbacks().setOnConnection([](std::uint32_t clientId) {
        std::cout << "Client connected: " << clientId << std::endl;
    });

    server.callbacks().setOnDisconnection([](std::uint32_t clientId) {
        std::cout << "Client disconnected: " << clientId << std::endl;
    });

    server.callbacks().setOnMessageReception([](glnet::connection::Type type, std::uint32_t clientId, glnet::Message& message) {
        std::string receivedMessage(message.payload.begin(), message.payload.end());
        std::cout << "Received message from client " << clientId << " via "
                  << (type == glnet::connection::Type::TCP ? "TCP" : "UDP")
                  << ": " << receivedMessage << std::endl;
    });

    while (!sigintCatch) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}

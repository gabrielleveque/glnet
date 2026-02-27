
#include "../include/Data/Packet.hpp"
#include "../include/Manager.hpp"
#include "../shared/Message.hpp"
#include "../shared/Player.hpp"

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

    // Set up callbacks for both connections and disconnections
    server.callbacks().setOnConnection([](std::uint32_t clientId) {
        std::cout << "Client connected: " << clientId << std::endl;
    });

    server.callbacks().setOnDisconnection([](std::uint32_t clientId) {
        std::cout << "Client disconnected: " << clientId << std::endl;
    });

    server.callbacks().setOnMessageReception([](glnet::connection::Type type, std::uint32_t clientId, glnet::Packet& packet) {
        if (type == glnet::connection::Type::TCP) {
            std::cout << "Received TCP message from client " << clientId << " with length " << packet.length << std::endl;
        } else {
            std::cout << "Received UDP message from client " << clientId << " with length " << packet.length << std::endl;
        }

        glnet::message::Type packetType;

        packet >> packetType;
        if (packetType == glnet::message::Type::PLAYER_POSITION) {
            Player::Position position = {0};

            packet >> position;
            std::cout << "Received player position from client " << clientId << ": (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
        if (packetType == glnet::message::Type::CHAT_MESSAGE) {
            std::string message;

            packet >> message;
            std::cout << "Received chat message from client " << clientId << ": " << message << std::endl;
        }
        std::cout << std::endl;
    });

    while (!sigintCatch) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Server shutting down..." << std::endl;
    return 0;
}

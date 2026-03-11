
#include "../include/Data/Packet.hpp"
#include "../include/Manager.hpp"
#include "../shared/Message.hpp"
#include "../shared/Player.hpp"

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
        std::cout << "Client connected to server. " << std::endl;

        // Send a position to the server
        glnet::Packet positionPacket;
        Player::Position position{10.05f, 20.02f, 5.0f};

        positionPacket << glnet::message::Type::PLAYER_POSITION << position;
        std::cout << positionPacket << std::endl;
        client.sendToServer(glnet::connection::Type::UDP, positionPacket);

        // Send a plain text message
        glnet::Packet messagePacket;

        messagePacket << glnet::message::Type::CHAT_MESSAGE << "Push the B site !";

        std::cout << messagePacket << std::endl;
        client.sendToServer(glnet::connection::Type::UDP, messagePacket);

    });

    client.connectToServer();

    while (!sigintCatch) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}

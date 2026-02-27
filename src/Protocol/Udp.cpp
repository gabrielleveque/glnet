
#include "Manager.hpp"
#include "Protocol/Udp.hpp"
#include "Utils/Converter.hpp"

#include <iostream>

glnet::Udp::Udp(Endpoint endpoint, connection::Side side) : side_(side), running_(true), socket_(connection::Type::UDP, endpoint)
{
    Socket::Address_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(endpoint.port);
    if (endpoint != Endpoint{"", 0}) {
        socket_.reuse();
        socket_.bind((Socket::Address&) addr, sizeof(addr));
    }
    pollFds_.push_back({.fd = socket_.getFd(), .events = POLLIN, .revents = 0});
}

void glnet::Udp::stop()
{
    running_ = false;
}

void glnet::Udp::run()
{
    try {
        while (running_) {
            std::int32_t result = socket_.poll(pollFds_, pollFds_.size(), 0);

            if (result > 0 && pollFds_.size() >= 1) {
                if (pollFds_[0].revents & POLLIN) {
                    readFromSocket();
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::size_t glnet::Udp::readDatagram(Socket::Address& addr, Socket::AddressLength& len, Packet& packet)
{
    std::vector<std::uint8_t> buffer(1024);
    std::size_t bytesRead = socket_.recvFrom(buffer.data(), buffer.size(), 0, addr, len);

    std::cout << "Received datagram with length " << bytesRead << std::endl;
    if (bytesRead < 5) {
        return 0;
    }
    std::memcpy(&packet.length, buffer.data(), sizeof(packet.length));
    packet.bytes = std::vector(buffer.begin() + sizeof(packet.length), buffer.begin() + sizeof(packet.length) + packet.length);
    return bytesRead;
}

void glnet::Udp::readFromSocket()
{
    try {
        Manager& manager = Manager::getInstance();
        Socket::Address addr = {0};
        Socket::AddressLength len = sizeof(addr);
        Endpoint endpoint = {.address = "", .port = 0};
        Packet packet;

        if (readDatagram(addr, len, packet) == 0) {
            return;
        }
        endpoint.address = ::inet_ntoa(((Socket::Address_in&) addr).sin_addr);
        endpoint.port = ntohs(((Socket::Address_in&) addr).sin_port);
        manager.callbackHandler(Callback::Type::ON_MESSAGE_RECEPTION, connection::Type::UDP, manager.getClientIdBy<Endpoint>(endpoint), packet);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void glnet::Udp::sendToEndpoint(Endpoint endpoint, Packet& packet)
{
    try {
        Socket::Address_in servAddr = {0};
        std::uint8_t buffer[sizeof(packet.length) + packet.length];

        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(endpoint.port);
        servAddr.sin_addr.s_addr = inet_addr(endpoint.address.c_str());

        std::memcpy(buffer, &packet.length, sizeof(packet.length));
        std::memcpy(buffer + sizeof(packet.length), packet.bytes.data(), packet.length);

        std::cout << "Sending datagram with length " << sizeof(packet.length) + packet.length << std::endl;

        socket_.sendTo(buffer, sizeof(packet.length) + packet.length, 0, (const Socket::Address&) servAddr, sizeof(servAddr));
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

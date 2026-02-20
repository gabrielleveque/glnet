
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

std::size_t glnet::Udp::readDatagram(Socket::Address& addr, Socket::AddressLength& len, Datagram& datagram)
{
    std::vector<std::uint8_t> buffer(1024);
    std::size_t bytesRead = socket_.recvFrom(buffer, buffer.size(), 0, addr, len);

    if (bytesRead < 5) {
        return 0;
    }
    datagram.opcode = buffer[0];
    datagram.length = utils::Converter::bytesToNumber(std::vector(buffer.begin() + 1, buffer.end()), 4);
    datagram.payload = std::vector(buffer.begin() + 5, buffer.begin() + 5 + datagram.length);
    return bytesRead;
}

void glnet::Udp::readFromSocket()
{
    try {
        Manager& manager = Manager::getInstance();
        Socket::Address addr = {0};
        Socket::AddressLength len = sizeof(addr);
        Endpoint endpoint = {.address = "", .port = 0};
        Datagram datagram = {0};

        if (readDatagram(addr, len, datagram) == 0) {
            return;
        }
        endpoint.address = ::inet_ntoa(((Socket::Address_in&) addr).sin_addr);
        endpoint.port = ntohs(((Socket::Address_in&) addr).sin_port);
        manager.callbackHandler(Callback::Type::ON_MESSAGE_RECEPTION, connection::Type::UDP, manager.getClientIdBy<Endpoint>(endpoint), datagram);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void glnet::Udp::sendToEndpoint(Endpoint endpoint, Buffer& msg)
{
    try {
        Socket::Address_in servAddr = {0};

        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(endpoint.port);
        servAddr.sin_addr.s_addr = inet_addr(endpoint.address.c_str());
        socket_.sendTo(msg.data, msg.data.size(), 0, (const Socket::Address&) servAddr, sizeof(servAddr));
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

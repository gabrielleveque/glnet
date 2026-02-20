
#include "Utils/Converter.hpp"
#include "Manager.hpp"
#include "Protocol/Tcp.hpp"

#include <iostream>
#include <thread>

glnet::Tcp::Tcp(Endpoint endpoint, connection::Side side) : side_(side), running_(true), socket_(connection::Type::TCP, endpoint)
{
    Socket::Address_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(endpoint.port);
    if (endpoint != Endpoint{"", 0}) {
        socket_.reuse();
        socket_.bind((Socket::Address&) addr, sizeof(addr));
        if (side_ == connection::Side::SERVER) {
            socket_.listen();
        }
    }
    pollFds_.push_back({.fd = socket_.getFd(), .events = POLLIN, .revents = 0});
}

void glnet::Tcp::stop()
{
    running_ = false;
}

void glnet::Tcp::run()
{
    try {
        Manager& manager = Manager::getInstance();

        while (running_) {
            std::int32_t result = socket_.poll(pollFds_, pollFds_.size(), 0);

            if (result > 0 && pollFds_.size() >= 1) {
                if (pollFds_[0].revents & POLLIN) {
                    if (side_ == connection::Side::SERVER) {
                        acceptSocket();
                    } else {
                        readFromSocket(socket_);
                    }
                }
                if (side_ == connection::Side::SERVER) {
                    for (std::size_t i = 1; i < pollFds_.size(); i++) {
                        if (pollFds_[i].revents & POLLHUP) {
                            disconnectSocket(i);
                            i--;
                            continue;
                        }
                        if (pollFds_[i].revents & POLLIN) {
                            if (!readFromSocket(manager.getClientSocketBy<Socket::Fd>(pollFds_[i].fd))) {
                                disconnectSocket(i);
                                i--;
                            }
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void glnet::Tcp::connectToServer(const std::string& host, std::uint16_t port)
{
    if (side_ != connection::Side::CLIENT) {
        return;
    }
    try {
        Manager& manager = Manager::getInstance();
        Socket::Address_in addr = {0};

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = Socket::inetAddr(host.c_str());
        socket_.connect((Socket::Address&) addr, sizeof(addr));
        manager.callbackHandler(Callback::Type::ON_CONNECTION, socket_);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void glnet::Tcp::acceptSocket()
{
    if (side_ != connection::Side::SERVER) {
        return;
    }
    try {
        Manager& manager = Manager::getInstance();
        Socket::Address addr = {0};
        Socket::AddressLength addrLen = sizeof(addr);
        Socket socket = socket_.accept(addr, addrLen);

        pollFds_.push_back({.fd = socket.getFd(), .events = POLLIN, .revents = 0});
        socket.setEndpoint({::inet_ntoa(((Socket::Address_in&) addr).sin_addr), ntohs(((Socket::Address_in&) addr).sin_port)});
        manager.callbackHandler(Callback::Type::ON_CONNECTION, socket);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void glnet::Tcp::disconnectSocket(std::size_t id)
{
    if (side_ != connection::Side::SERVER) {
        return;
    }
    try {
        Manager& manager = Manager::getInstance();
        Socket& socket = manager.getClientSocketBy<Socket::Fd>(pollFds_[id].fd);

        manager.callbackHandler(Callback::Type::ON_DISCONNECTION, manager.getClientIdBy<Socket>(socket));
        pollFds_.erase(pollFds_.begin() + id);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::size_t glnet::Tcp::readHeader(Socket& socket, Segment& segment)
{
    std::vector<std::uint8_t> buffer(5);
    std::size_t bytesRead = socket.recv(buffer, 5, 0);

    if (bytesRead != 5) {
        return 0;
    }
    segment.opcode = buffer[0];
    segment.length = utils::Converter::bytesToNumber(std::vector(buffer.begin() + 1, buffer.end()), 4);
    return bytesRead;
}

std::size_t glnet::Tcp::readBody(Socket& socket, Segment& segment)
{
    if (segment.length == 0) {
        return 0;
    }
    segment.payload.resize(segment.length);
    std::size_t bytesRead = socket.recv(segment.payload, segment.length, 0);

    if (bytesRead != segment.length) {
        return 0;
    }
    return bytesRead;
}

bool glnet::Tcp::readFromSocket(Socket& socket)
{
    if (!running_) {
        return false;
    }
    try {
        Manager& manager = Manager::getInstance();
        Segment segment = {0};

        if (readHeader(socket, segment) == 0) {
            return false;
        }
        if (readBody(socket, segment) == 0 && segment.length != 0) {
            return false;
        }
        manager.callbackHandler(Callback::Type::ON_MESSAGE_RECEPTION, connection::Type::TCP, manager.getClientIdBy<Socket>(socket), segment);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return true;
}

void glnet::Tcp::sendToSocket(Socket& socket, Buffer& msg)
{
    try {
        socket.send(msg.data, msg.data.size(), 0);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

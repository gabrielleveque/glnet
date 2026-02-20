
#include "Socket.hpp"

#ifdef _WIN32

#else
#include <errno.h>
#endif

#include <cstring>
#include <format>

glnet::Socket::Socket(connection::Type type, Endpoint endpoint) : endpoint_(endpoint), isOwner_(true)
{
    if (type == connection::Type::TCP) {
        fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (type == connection::Type::UDP) {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (fd_ == INVALID_FD) {
        throw std::runtime_error(std::format("Couldn't create the socket: {}.", getLastError()));
    }
}

glnet::Socket::Socket(Fd fd, bool isOwner) : fd_(fd), isOwner_(isOwner)
{
    if (fd == INVALID_FD) {
        throw std::runtime_error(std::format("Invalid file descriptor provided."));
    }
    startup();
}

glnet::Socket::~Socket()
{
    if (!isOwner_) {
        return;
    }
    if (fd_ != INVALID_FD) {
#ifdef _WIN32
        closesocket(fd_);
#else
        ::close(fd_);
#endif
    }
}

void glnet::Socket::startup()
{
#ifdef _WIN32
    WSADATA wsaData = {0};

    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error(std::format("WSAStartup failed: {}.", getLastError()));
    }
#endif
}

void glnet::Socket::cleanup()
{
#ifdef _WIN32
    ::WSACleanup();
#endif
}

void glnet::Socket::bind(const Address& addr, AddressLength addrLen)
{
    if (::bind(fd_, &addr, addrLen) == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Couldn't bind the socket: {}.", getLastError()));
    }
}

void glnet::Socket::listen(std::int32_t backlog)
{
    if (::listen(fd_, backlog) == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Couldn't listen on the socket: {}.", getLastError()));
    }
}

void glnet::Socket::reuse(bool enable)
{
    std::uint32_t opt = 1;

#ifdef _WIN32
    if (!enable) {
        if (::setsockopt(fd_, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &opt, sizeof(opt)) == SOCKET_ERROR_CODE) {
            throw std::runtime_error(std::format("Couldn't set the exclusive address use option on the socket: {}.", getLastError()));
        }
    } else {
        if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) == SOCKET_ERROR_CODE) {
            throw std::runtime_error(std::format("Couldn't set the reuse address option on the socket: {}.", getLastError()));
        }
    }
#else
    if (!enable) {
        return;
    }
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR_CODE || ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Couldn't set the reuse options on the socket: {}.", getLastError()));
    }
#endif
}

glnet::Socket glnet::Socket::accept(OptionalReference<Address> addr, OptionalReference<AddressLength> addrLen)
{
    Address *addrPtr = addr.has_value() ? &addr.value().get() : nullptr;
    AddressLength *addrLenPtr = addrLen.has_value() ? &addrLen.value().get() : nullptr;
    Fd clientFd = 0;

    clientFd = ::accept(fd_, addrPtr, addrLenPtr);
    if (clientFd == INVALID_FD) {
        throw std::runtime_error(std::format("Couldn't accept the connection: {}.", getLastError()));
    }
    return clientFd;
}

void glnet::Socket::connect(const Address& addr, AddressLength addrLen)
{
    if (::connect(fd_, &addr, addrLen) == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Couldn't connect to the address: {}.", getLastError()));
    }
}

std::int32_t glnet::Socket::poll(std::vector<PollFd>& fds, NFDS nfds, std::int32_t timeout)
{
    std::int32_t polled = 0;

#ifdef _WIN32
    polled = ::WSAPoll(fds.data(), nfds, timeout);
#else
    polled = ::poll(fds.data(), nfds, timeout);
#endif
    if (polled == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Poll error on the socket: {}.", getLastError()));
    }
    return polled;
}

glnet::Socket::BytesSent glnet::Socket::send(const Buffer& buffer, BufferLength length, std::int32_t flags)
{
    BytesSent bytesSent = 0;

    bytesSent = ::send(fd_, (const char *) buffer.data(), length, flags);
    if (bytesSent == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Send error on the socket: {}.", getLastError()));
    }
    return bytesSent;
}

glnet::Socket::BytesReceived glnet::Socket::recv(Buffer& buffer, BufferLength length, std::int32_t flags)
{
    BytesReceived bytesReceived = 0;

    bytesReceived = ::recv(fd_, (char *) buffer.data(), length, flags);
    if (bytesReceived == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Receive error on the socket: {}.", getLastError()));
    }
    return bytesReceived;
}

glnet::Socket::BytesReceived glnet::Socket::sendTo(const Buffer& buffer, BufferLength length, std::int32_t flags, const Address& destAddr, AddressLength destAddrLen)
{
    BytesSent bytesSent = 0;

    bytesSent = ::sendto(fd_, (const char *) buffer.data(), length, flags, &destAddr, destAddrLen);
    if (bytesSent == SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Send error to an endpoint: {}.", getLastError()));
    }
    return bytesSent;
}

glnet::Socket::BytesReceived glnet::Socket::recvFrom(
    Buffer& buffer, BufferLength length, std::int32_t flags, OptionalReference<Address> srcAddr, OptionalReference<AddressLength> srcAddrLen)
{
    Address *addrPtr = srcAddr.has_value() ? &srcAddr->get() : nullptr;
    AddressLength *addrLenPtr = srcAddrLen.has_value() ? &srcAddrLen->get() : nullptr;
    BytesReceived bytesReceived = 0;

    bytesReceived = ::recvfrom(fd_, (char *) buffer.data(), length, flags, addrPtr, addrLenPtr);
    if (bytesReceived == SOCKET_ERROR_CODE) {

        throw std::runtime_error(std::format("Receive error from an endpoint: {}.", getLastError()));
    }
    return bytesReceived;
}

std::int32_t glnet::Socket::getSockName(Address& addr, AddressLength& addrLen)
{
    if (::getsockname(fd_, &addr, &addrLen) < SOCKET_ERROR_CODE) {
        throw std::runtime_error(std::format("Error getting socket name: {}.", getLastError()));
    }
    return 0;
}

glnet::Socket::Fd glnet::Socket::getFd() const
{
    return fd_;
}

glnet::Endpoint glnet::Socket::getEndpoint() const
{
    return endpoint_;
}

void glnet::Socket::setEndpoint(Endpoint endpoint)
{
    endpoint_ = endpoint;
}

std::string glnet::Socket::getLastError()
{
    std::string error;

#ifdef _WIN32
    error = std::to_string(::WSAGetLastError());
#else
    error = std::string(std::strerror(errno));
#endif
    return error;
}

glnet::Socket::InAddr glnet::Socket::inetAddr(std::string ip)
{
    return ::inet_addr(ip.c_str());
}

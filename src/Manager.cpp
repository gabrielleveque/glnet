
#include "Manager.hpp"
#include "Utils/Threads.hpp"

#include <type_traits>

glnet::Manager::Manager() : running_(true)
{
    Socket::startup();
}

glnet::Manager::~Manager()
{
    running_ = false;
    if (tcp_) {
        tcp_->stop();
    }
    utils::Threads::join(tcpThread_);
    if (udp_) {
        udp_->stop();
    }
    utils::Threads::join(udpThread_);
    utils::Threads::join(mainThread_);
    Socket::cleanup();
}

void glnet::Manager::initialize(connection::Side side)
{
    if (side == connection::Side::CLIENT) {
        client_.clientPort = getAvailablePort();
    }
    side_ = side;
    mainThread_ = std::thread(&Manager::run, this);
}

void glnet::Manager::stop()
{
    running_ = false;
}

void glnet::Manager::run()
{
    while (running_) {
        while (!disconnectionQueue_.empty()) {
            if (side_ == connection::Side::SERVER) {
                std::uint32_t id = disconnectionQueue_.front();

                if (server_.clients.find(id) == server_.clients.end()) {
                    return;
                }
                callbacks_.onDisconnection(id);
                server_.clients.erase(id);
            }
            disconnectionQueue_.pop();
        }
    }
}

void glnet::Manager::createConnection(connection::Type type, Endpoint endpoint)
{
    if (side_ == connection::Side::CLIENT) {
        endpoint.port = client_.clientPort;
    }
    switch (type) {
        case connection::Type::TCP:
            tcp_ = std::make_shared<Tcp>(endpoint, side_);
            if (side_ == connection::Side::SERVER) {
                tcpThread_ = std::thread(&Tcp::run, tcp_);
            }
            break;
        case connection::Type::UDP:
            udp_ = std::make_shared<Udp>(endpoint, side_);
            udpThread_ = std::thread(&Udp::run, udp_);
            break;
        default:
            break;
    }
}

void glnet::Manager::connectToServer()
{
    if (side_ == connection::Side::CLIENT && tcp_) {
        tcp_->connectToServer(client_.server.address, client_.server.port);
        tcpThread_ = std::thread(&Tcp::run, tcp_);
    }
}

void glnet::Manager::sendMessage(connection::Type type, Buffer msg)
{
    if (side_ != connection::Side::CLIENT) {
        return;
    }
    switch (type) {
        case connection::Type::TCP:
            tcp_->sendToSocket(*client_.socket, msg);
            break;
        case connection::Type::UDP:
            udp_->sendToEndpoint(client_.server, msg);
            break;
        default:
            break;
    }
}

void glnet::Manager::sendMessageTo(connection::Type type, std::vector<std::uint32_t> ids, Buffer msg)
{
    if (side_ != connection::Side::SERVER || ids.empty()) {
        return;
    }
    for (std::uint32_t id : ids) {
        if (server_.clients.find(id) == server_.clients.end()) {
            return;
        }
        switch (type) {
            case connection::Type::TCP:
                tcp_->sendToSocket(*server_.clients[id], msg);
                break;
            case connection::Type::UDP:
                if (!server_.clients[id]) {
                    return;
                }
                udp_->sendToEndpoint(server_.clients[id]->getEndpoint(), msg);
                break;
            default:
                break;
        }
    }
}

void glnet::Manager::callbackHandler(Callback::Type callback, Socket& socket)
{
    if (callback == Callback::Type::ON_CONNECTION) {
        std::shared_ptr<Socket> connectionSocket = std::make_shared<Socket>(socket);

        if (side_ == connection::Side::CLIENT) {
            client_.socket = connectionSocket;
            callbacks_.onConnection(0);
        } else if (side_ == connection::Side::SERVER) {
            server_.clients[server_.nextClientId] = connectionSocket;
            callbacks_.onConnection(server_.nextClientId);
            server_.nextClientId++;
        }
    }
}

void glnet::Manager::callbackHandler(Callback::Type callback, std::uint32_t id)
{
    if (callback == Callback::Type::ON_DISCONNECTION) {
        disconnectionQueue_.push(id);
    }
}

void glnet::Manager::callbackHandler(Callback::Type callback, connection::Type type, std::uint32_t id, Message& message)
{
    if (callback == Callback::Type::ON_MESSAGE_RECEPTION) {
        if (side_ != connection::Side::CLIENT && server_.clients.find(id) == server_.clients.end()) {
            return;
        }
        callbacks_.onMessageReception(type, id, message);
    }
}

template <typename T>
glnet::Socket& glnet::Manager::getClientSocketBy(T& ref)
{
    if (side_ == connection::Side::CLIENT) {
        throw std::runtime_error("Client side has no clients");
    }
    if (std::is_same_v<T, Socket::Fd>) {
        for (auto& [id, client] : server_.clients) {
            if (client && client->getFd() == ref) {
                return *client;
            }
        }
    }
    throw std::runtime_error("Client not found");
}

template glnet::Socket& glnet::Manager::getClientSocketBy<glnet::Socket::Fd>(glnet::Socket::Fd& ref);

template <typename T>
std::uint32_t glnet::Manager::getClientIdBy(T& ref)
{
    if (side_ == connection::Side::CLIENT) {
        return 0;
    }
    if constexpr (std::is_same_v<T, Socket>) {
        for (auto& [id, client] : server_.clients) {
            if (client && client->getFd() == ref.getFd()) {
                return id;
            }
        }
    } else if constexpr (std::is_same_v<T, Endpoint>) {
        for (auto& [id, client] : server_.clients) {
            if (client && client->getEndpoint() == ref) {
                return id;
            }
        }
    }
    throw std::runtime_error("Client not found");
}

template std::uint32_t glnet::Manager::getClientIdBy<glnet::Socket>(glnet::Socket& ref);
template std::uint32_t glnet::Manager::getClientIdBy<glnet::Endpoint>(glnet::Endpoint& ref);

void glnet::Manager::setServerEndpoint(Endpoint endpoint)
{
    client_.server = endpoint;
}

std::uint16_t glnet::Manager::getAvailablePort()
{
    Socket socket(connection::Type::TCP, {LOCALHOST, 0});
    Endpoint endpoint = socket.getEndpoint();
    Socket::Address_in addr = {0};
    Socket::AddressLength addrLen = 0;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = ::inet_addr(LOCALHOST);
    addrLen = sizeof(addr);
    socket.bind((Socket::Address&) addr, addrLen);
    socket.getSockName((Socket::Address&) addr, addrLen);
    return ntohs(addr.sin_port);
}

glnet::Callback& glnet::Manager::callbacks()
{
    return callbacks_;
}

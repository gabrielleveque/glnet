
#include "Manager.hpp"
#include "Utils/Threads.hpp"

#include <type_traits>

glnet::Manager::Manager() : _running(true)
{
    Socket::startup();
}

glnet::Manager::~Manager()
{
    _running = false;
    if (_tcp) {
        _tcp->stop();
    }
    utils::Threads::join(_tcpThread);
    if (_udp) {
        _udp->stop();
    }
    utils::Threads::join(_udpThread);
    utils::Threads::join(_mainThread);
    Socket::cleanup();
}

void glnet::Manager::initialize(connection::Side side)
{
    if (side == connection::Side::CLIENT) {
        _client.clientPort = getAvailablePort();
    }
    _side = side;
    _mainThread = std::thread(&Manager::run, this);
}

void glnet::Manager::stop()
{
    _running = false;
}

void glnet::Manager::run()
{
    while (_running) {
        while (!_disconnectionQueue.empty()) {
            if (_side == connection::Side::SERVER) {
                std::uint32_t id = _disconnectionQueue.front();

                if (_server.clients.find(id) == _server.clients.end()) {
                    return;
                }
                _callbacks.onDisconnection(id);
                _server.clients.erase(id);
            }
            _disconnectionQueue.pop();
        }
    }
}

void glnet::Manager::createConnection(connection::Type type, Endpoint endpoint)
{
    if (_side == connection::Side::CLIENT) {
        endpoint.port = _client.clientPort;
    }
    switch (type) {
        case connection::Type::TCP:
            _tcp = std::make_shared<Tcp>(endpoint, _side);
            if (_side == connection::Side::SERVER) {
                _tcpThread = std::thread(&Tcp::run, _tcp);
            }
            break;
        case connection::Type::UDP:
            _udp = std::make_shared<Udp>(endpoint, _side);
            _udpThread = std::thread(&Udp::run, _udp);
            break;
        default:
            break;
    }
}

void glnet::Manager::connectToServer()
{
    if (_side == connection::Side::CLIENT && _tcp) {
        _tcp->connectToServer(_client.server.address, _client.server.port);
        _tcpThread = std::thread(&Tcp::run, _tcp);
    }
}

void glnet::Manager::sendMessage(connection::Type type, Buffer msg)
{
    if (_side != connection::Side::CLIENT) {
        return;
    }
    switch (type) {
        case connection::Type::TCP:
            _tcp->sendToSocket(*_client.socket, msg);
            break;
        case connection::Type::UDP:
            _udp->sendToEndpoint(_client.server, msg);
            break;
        default:
            break;
    }
}

void glnet::Manager::sendMessageTo(connection::Type type, std::vector<std::uint32_t> ids, Buffer msg)
{
    if (_side != connection::Side::SERVER || ids.empty()) {
        return;
    }
    for (std::uint32_t id : ids) {
        if (_server.clients.find(id) == _server.clients.end()) {
            return;
        }
        switch (type) {
            case connection::Type::TCP:
                _tcp->sendToSocket(*_server.clients[id], msg);
                break;
            case connection::Type::UDP:
                if (!_server.clients[id]) {
                    return;
                }
                _udp->sendToEndpoint(_server.clients[id]->getEndpoint(), msg);
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

        if (_side == connection::Side::CLIENT) {
            _client.socket = connectionSocket;
            _callbacks.onConnection(0);
        } else if (_side == connection::Side::SERVER) {
            _server.clients[_server.nextClientId] = connectionSocket;
            _callbacks.onConnection(_server.nextClientId);
            _server.nextClientId++;
        }
    }
}

void glnet::Manager::callbackHandler(Callback::Type callback, std::uint32_t id)
{
    if (callback == Callback::Type::ON_DISCONNECTION) {
        _disconnectionQueue.push(id);
    }
}

void glnet::Manager::callbackHandler(Callback::Type callback, connection::Type type, std::uint32_t id, Message& message)
{
    if (callback == Callback::Type::ON_MESSAGE_RECEPTION) {
        if (_side != connection::Side::CLIENT && _server.clients.find(id) == _server.clients.end()) {
            return;
        }
        _callbacks.onMessageReception(type, id, message);
    }
}

template <typename T>
glnet::Socket& glnet::Manager::getClientSocketBy(T& ref)
{
    if (_side == connection::Side::CLIENT) {
        throw std::runtime_error("Client side has no clients");
    }
    if (std::is_same_v<T, Socket::Fd>) {
        for (auto& [id, client] : _server.clients) {
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
    if (_side == connection::Side::CLIENT) {
        return 0;
    }
    if constexpr (std::is_same_v<T, Socket>) {
        for (auto& [id, client] : _server.clients) {
            if (client && client->getFd() == ref.getFd()) {
                return id;
            }
        }
    } else if constexpr (std::is_same_v<T, Endpoint>) {
        for (auto& [id, client] : _server.clients) {
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
    _client.server = endpoint;
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
    return _callbacks;
}

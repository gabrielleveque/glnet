
#include "Callback.hpp"

void glnet::Callback::onConnection(std::uint32_t clientId)
{
    if (_onConnection) {
        _onConnection(clientId);
    }
}

void glnet::Callback::setOnConnection(std::function<void(std::uint32_t)> func)
{
    _onConnection = func;
}

void glnet::Callback::onDisconnection(std::uint32_t clientId)
{
    if (_onDisconnection) {
        _onDisconnection(clientId);
    }
}

void glnet::Callback::setOnDisconnection(std::function<void(std::uint32_t)> func)
{
    _onDisconnection = func;
}

void glnet::Callback::onMessageReception(connection::Type type, std::uint32_t clientId, Message& message)
{
    if (_onMessageReception) {
        _onMessageReception(type, clientId, message);
    }
}

void glnet::Callback::setOnMessageReception(std::function<void(connection::Type, std::uint32_t, Message&)> func)
{
    _onMessageReception = func;
}

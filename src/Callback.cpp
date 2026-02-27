
#include "Callback.hpp"

void glnet::Callback::onConnection(std::uint32_t clientId)
{
    if (onConnection_) {
        onConnection_(clientId);
    }
}

void glnet::Callback::setOnConnection(std::function<void(std::uint32_t)> func)
{
    onConnection_ = func;
}

void glnet::Callback::onDisconnection(std::uint32_t clientId)
{
    if (onDisconnection_) {
        onDisconnection_(clientId);
    }
}

void glnet::Callback::setOnDisconnection(std::function<void(std::uint32_t)> func)
{
    onDisconnection_ = func;
}

void glnet::Callback::onMessageReception(connection::Type type, std::uint32_t clientId, Packet& packet)
{
    if (onMessageReception_) {
        onMessageReception_(type, clientId, packet);
    }
}

void glnet::Callback::setOnMessageReception(std::function<void(connection::Type, std::uint32_t, Packet&)> func)
{
    onMessageReception_ = func;
}


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

void glnet::Callback::onMessageReception(connection::Type type, std::uint32_t clientId, Message& message)
{
    if (onMessageReception_) {
        onMessageReception_(type, clientId, message);
    }
}

void glnet::Callback::setOnMessageReception(std::function<void(connection::Type, std::uint32_t, Message&)> func)
{
    onMessageReception_ = func;
}

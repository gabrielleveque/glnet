#include "Utils/Converter.hpp"
#include "Data/Packet.hpp"

glnet::Packet::Packet() : length(0), writeOffset_(0), readOffset_(0)
{
}

glnet::Packet& glnet::Packet::operator<<(const std::string data)
{
    if (data.size() > UINT16_MAX) {
        throw std::runtime_error(std::format("An std::string cannot be stored in a packet with a size higher than {}", UINT16_MAX));
    }
    std::vector<std::uint8_t> header = utils::Converter::numberToBytes(data.size(), 2);

    bytes.insert(bytes.end(), header.begin(), header.end());
    bytes.insert(bytes.end(), data.begin(), data.end());

    writeOffset_ += data.size() + STD_STRING_HEADER_SIZE;
    length += data.size() + STD_STRING_HEADER_SIZE;
    return *this;
}

glnet::Packet& glnet::Packet::operator<<(const char *data)
{
    return (*this << std::string(data));
}

glnet::Packet& glnet::Packet::operator>>(std::string& data)
{
    std::uint16_t stringSize = utils::Converter::bytesToNumber(std::vector<std::uint8_t>(bytes.begin() + readOffset_, bytes.end()), 2);

    data = std::string(bytes.begin() + readOffset_ + STD_STRING_HEADER_SIZE, bytes.begin() + readOffset_ + STD_STRING_HEADER_SIZE + stringSize);

    readOffset_ += STD_STRING_HEADER_SIZE + stringSize;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const glnet::Packet& packet)
{
    out << "There are " << packet.bytes.size() << " bytes in the given packet." << '\n';
    for (const std::uint8_t& byte : packet.bytes) {
        out << (int) byte << ' ';
    }
    return out;
}

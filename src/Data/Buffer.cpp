
#include "Utils/Converter.hpp"
#include "Data/Buffer.hpp"

glnet::Buffer::Buffer(const char *data_)
{
    if (data_) {
        constructData(std::string(data_));
    }
}

glnet::Buffer::Buffer(std::string data_)
{
    constructData(data_);
}

glnet::Buffer::Buffer(std::vector<std::uint8_t> data_) : data(data_)
{
}

glnet::Buffer::Buffer(std::initializer_list<std::uint8_t> data_) : data(std::vector<std::uint8_t>(data_))
{
}

#include <iostream>

void glnet::Buffer::constructData(std::string data_)
{
    std::uint32_t length = data_.size();
    std::vector<std::uint8_t> lengthBytes = utils::Converter::numberToBytes(length, 4);

    data.reserve(HEADER_SIZE + length);
    data.push_back(DEFAULT_OPCODE);
    data.insert(data.end(), lengthBytes.begin(), lengthBytes.end());
    data.insert(data.end(), data_.begin(), data_.end());
}

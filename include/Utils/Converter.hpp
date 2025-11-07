
#pragma once

#include <cstdint>
#include <vector>

namespace glnet::utils
{
    class Converter
    {
        public:
            /**
             * @brief Convert a number into an array of bytes
             *
             * @param number The number to convert
             * @param byteToWrite The number of bytes to write the number on
             * @return std::vector<std::uint8_t> The converted number
             */
            static std::vector<std::uint8_t> numberToBytes(const std::uint64_t number, std::uint8_t byteToWrite);

            /**
             * @brief Convert an array of bytes into a number
             *
             * @param bytes The array of bytes to convert
             * @param byteToRead The number of bytes to read from the array
             * @return std::uint64_t The converted number
             */
            static std::uint64_t bytesToNumber(const std::vector<std::uint8_t> bytes, std::uint8_t byteToRead);
    };
}


#pragma once

#include <initializer_list>
#include <cstdint>
#include <vector>

#define HEADER_SIZE 5
#define DEFAULT_OPCODE 0

namespace glnet
{
    class Buffer
    {
        public:
            /**
             * @brief Construct a new Buffer object
             *
             * @param str The str to store in the buffer
             */
            Buffer(const char *data_);
            /**
             * @brief Construct a new Buffer object
             *
             * @param str The str to store in the buffer
             */
            Buffer(std::string data_);

            /**
             * @brief Construct a new Buffer object
             *
             * @param list The list to store in the buffer
             */
            Buffer(std::vector<std::uint8_t> data_);

            /**
             * @brief Construct a new Buffer object
             *
             * @param list The list to store in the buffer
             */
            Buffer(std::initializer_list<std::uint8_t> data_);

            std::vector<std::uint8_t> data; /*!< The data stored in the buffer */

        private:
            /** @brief Construct data from string
             *
             * @param data_ The string to construct data from
             */
            void constructData(std::string data_);
    };
}

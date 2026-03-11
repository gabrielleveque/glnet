
#pragma once

#include <vector>
#include <iostream>

namespace glnet
{
    constexpr std::uint8_t STD_STRING_HEADER_SIZE = 2; /*!> The size of the header for a std::string in bytes */

    class Packet
    {
        public:
            /**
             * @brief Construct a new Packet object
             */
            Packet();

            std::uint32_t length;            /*!> The length of the packet (not including the header) */
            std::vector<std::uint8_t> bytes; /*!> A vector of bytes to hold the data */

            /**
             * @brief Overload of the insertion operator for trivially copyable types, which appends the binary representation of the data to the packet's byte vector
             *
             * @tparam T The type of the data to be added to the packet, which must be trivially copyable
             * @param data The data to be added to the packet
             * @return Packet& A reference to the packet with the data added
             */
            template <typename T>
            Packet& operator<<(const T data)
            {
                static_assert(std::is_trivially_copyable_v<T>, "Payload type must be trivially copyable");

                bytes.resize(bytes.size() + sizeof(T));
                std::memcpy(bytes.data() + writeOffset_, &data, sizeof(T));

                writeOffset_ += sizeof(T);
                length += sizeof(T);
                return *this;
            };

            /**
             * @brief Overload of the insertion operator for std::string, which handles the string length as well as the string content
             *
             * @param data The std::string to be added to the packet
             * @return Packet& A reference to the packet with the string data added
             */
            Packet& operator<<(const std::string data);

            /**
             * @brief Overload of the insertion operator for C-style strings, which converts the C-style string to a std::string and then handles it as a std::string
             *
             * @param data The C-style string to be added to the packet
             * @return Packet& A reference to the packet with the string data added
             */
            Packet& operator<<(const char *data);

            /**
             * @brief Overload of the extraction operator for trivially copyable types, which reads the binary representation of the data from the packet's byte vector and stores it in the provided
             * variable
             *
             * @tparam T The type of the data to be extracted from the packet, which must be trivially copyable
             * @param data A reference to the variable where the extracted data will be stored
             * @return Packet& A reference to the packet after the data has been extracted
             */
            template <typename T>
            Packet& operator>>(T& data)
            {
                static_assert(std::is_trivially_copyable_v<T>, "Payload type must be trivially copyable");

                if (bytes.size() < sizeof(T)) {
                    throw std::runtime_error("Insufficient data to transform into the target type");
                }

                if constexpr (std::is_pointer<T>::value) {
                    data = reinterpret_cast<T>(bytes.data());
                } else {
                    std::memcpy(&data, bytes.data() + readOffset_, sizeof(T));
                }
                readOffset_ += sizeof(T);
                return *this;
            };

            /**
             * @brief Overload of the extraction operator for std::string, which reads the string length and content from the packet's byte vector and stores it in the provided std::string variable
             *
             * @param data A reference to the std::string variable where the extracted string will be stored
             * @return Packet& A reference to the packet after the string data has been extracted
             */
            Packet& operator>>(std::string& data);

        private:
            std::uint32_t writeOffset_; /*!> The offset used when writing the bytes  */
            std::uint32_t readOffset_;  /*!> The offset used when reading the bytes  */
    };
}
std::ostream& operator<<(std::ostream& out, const glnet::Packet& packet);

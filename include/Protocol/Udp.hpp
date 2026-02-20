
#pragma once

#include "Enum/Connection.hpp"
#include "Data/Endpoint.hpp"
#include "Data/Message.hpp"
#include "Data/Buffer.hpp"
#include "Socket.hpp"

#include <cstdint>
#include <thread>

namespace glnet
{
    class Udp
    {
        public:
            /**
             * @brief Construct a new Udp object
             *
             * @param endpoint The endpoint on which to create the object
             */
            Udp(Endpoint endpoint, connection::Side side);

            /**
             * @brief Construct a new Udp object
             */
            Udp();

            /**
             * @brief Stop the udp instance
             */
            void stop();

            /**
             * @brief Main loop of the udp instance
             */
            void run();

            /**
             * @brief Read from the udp socket
             */
            void readFromSocket();

            /**
             * @brief Send a message to a given socket
             *
             * @param endpoint The endpoint where to send the message
             * @param msg The message to send
             */
            void sendToEndpoint(Endpoint endpoint, Buffer& msg);

        private:
            /**
             * @brief Read the header of the datagram issued to the server
             *
             * @param endpoint The endpoint to read from
             * @param datagram The datagram to store the information in
             * @return std::size_t The number of bytes read
             */
            std::size_t readDatagram(Socket::Address& addr, Socket::AddressLength& len, Datagram& datagram);

            connection::Side side_; /*!> The side of the connection (client or server) */
            bool running_;                /*!> If the tcp instance should run */

            Socket socket_;                       /*!> The udp socket */
            std::vector<Socket::PollFd> pollFds_; /*!> The pollfd array for the tcp instance */
    };
}

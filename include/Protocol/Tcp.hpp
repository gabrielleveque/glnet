
#pragma once

#include "Enum/Connection.hpp"
#include "Data/Endpoint.hpp"
#include "Data/Buffer.hpp"
#include "Data/Packet.hpp"
#include "Socket.hpp"

#include <iostream>
#include <cstdint>

namespace glnet
{
    class Tcp
    {
        public:
            /**
             * @brief Construct a new Tcp object
             *
             * @param endpoint The endpoint on which to create the object
             * @param type The side of the connection (client or server)
             */
            Tcp(Endpoint endpoint, connection::Side side);

            /**
             * @brief Stop the tcp instance
             */
            void stop();

            /**
             * @brief Main loop of the tcp instance
             */
            void run();

            /**
             * @brief Connect to a server
             *
             * @param host The host to connect to
             * @param port The port to connect to
             */
            void connectToServer(const std::string& host, std::uint16_t port);

            /**
             * @brief Send a packet to a given socket
             *
             * @param socket The socket to send to
             * @param packet The packet to send
             */
            void sendToSocket(Socket& socket, Packet& packet);

        private:
            connection::Side side_; /*!> The side of the connection (client or server) */
            bool running_;          /*!> If the tcp instance should run */

            Socket socket_;                       /*!> The tcp instance socket */
            std::vector<Socket::PollFd> pollFds_; /*!> The pollfd array for the tcp instance */

            /**
             * @brief Accept a socket on the tcp instance
             */
            void acceptSocket();

            /**
             * @brief Disconnect a socket from the tcp instance
             */
            void disconnectSocket(std::size_t id);

            /**
             * @brief Read the header of the segment issued to the server
             *
             * @param socket The socket to read on
             * @param packet The packet to store the information in
             * @return std::size_t The number of bytes read
             */
            std::size_t readHeader(Socket& socket, Packet& packet);

            /**
             * @brief Read the body of the segment issued to the server
             *
             * @param socket The socket to read on
             * @param packet The packet to store the information in
             * @return std::size_t The number of bytes read
             */
            std::size_t readBody(Socket& socket, Packet& packet);

            /**
             * @brief Read from a given socket
             *
             * @param socket The socket to read from
             * @param id The id of the client (for disconnection purpose)
             */
            bool readFromSocket(Socket& socket);
    };
}


#pragma once

#include "Enum/Connection.hpp"
#include "Utils/Singleton.hpp"
#include "Protocol/Tcp.hpp"
#include "Protocol/Udp.hpp"
#include "Data/Buffer.hpp"
#include "Data/Packet.hpp"
#include "Callback.hpp"

#include <unordered_map>
#include <functional>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>
#include <queue>

#define LOCALHOST "127.0.0.1"

namespace glnet
{
    class Manager : public utils::Singleton<Manager>
    {
        public:
            /**
             * @brief Construct a new Network Manager object
             */
            Manager();

            /**
             * @brief Destroy the Network Manager object
             */
            ~Manager();

            /**
             * @brief Initialize the Manager
             *
             * @param side The side of the connection (client or server)
             */
            void initialize(connection::Side side);

            /**
             * @brief Stop the Manager
             */
            void stop();

            /**
             * @brief Create a Connection object
             *
             * @param type The type of connection to create
             * @param endpoint The endpoint on which to create the connection
             */
            void createConnection(connection::Type type, Endpoint endpoint = Endpoint{"", 0});

            /**
             * @brief Connect to the server (only for client side)
             */
            void connectToServer();

            /**
             * @brief Send a packet to the server
             *
             * @param type The type of connection to use
             * @param packet The packet to send
             */
            void sendToServer(connection::Type type, Packet& packet);

            /**
             * @brief Send a packet to the clients
             *
             * @param type The type of connection to use
             * @param ids The ids of the clients to send to
             * @param packet The packet to send
             */
            void sendToClients(connection::Type type, std::vector<std::uint32_t> ids, Packet& packet);

            /**
             * @brief Handler of the callbacks
             *
             * @param callback The type of the callback
             * @param socket The socket of the client
             */
            void callbackHandler(Callback::Type callback, Socket& socket);

            /**
             * @brief Handler of the callbacks
             *
             * @param callback The type of the callback
             * @param id The id of the client
             */
            void callbackHandler(Callback::Type callback, std::uint32_t id);

            /**
             * @brief Handler of the callbacks
             *
             * @param callback The type of the callback
             * @param type The type of the connection
             * @param id The id of the client
             * @param message The received message
             */
            void callbackHandler(Callback::Type callback, connection::Type type, std::uint32_t id, Packet& packet);

            /**
             * @brief Get the Client Socket By object
             *
             * @tparam T The type of the reference
             * @param ref The reference to the object
             * @return Socket& The socket of the client corresponding to the given object
             */
            template <typename T>
            Socket& getClientSocketBy(T& ref);

            /**
             * @brief Get the Client Id By object
             *
             * @tparam T The type of the reference
             * @param ref The reference to the object
             * @return std::uint32_t The id of the client corresponding to the given object
             */
            template <typename T>
            std::uint32_t getClientIdBy(T& ref);

            /**
             * @brief Set the server endpoint (only for client side)
             *
             * @param endpoint The endpoint of the server
             */
            void setServerEndpoint(Endpoint endpoint);

            /**
             * @brief Get the Callback Handler object
             *
             * @return Callback& The callback handler
             */
            Callback& callbacks();

        private:
            /**
             * @struct Server
             * @brief Represent the server information
             */
            struct Server {
                    std::unordered_map<std::uint32_t, std::shared_ptr<Socket>> clients; /*!> The map of the clients connected via the tcp socket */
                    std::uint32_t nextClientId;                                         /*!> The next id to give to a client */
            } server_; /*!> The server information (only for server side) */

            /**
             * @struct Client
             * @brief Represent the client information
             */
            struct Client {
                    Endpoint server;          /*!> The endpoint of the server */
                    std::shared_ptr<Socket> socket; /*!> The client connection information */
                    std::uint32_t clientPort;       /*!> The port of the client */
            } client_;  /*!> The client information (only for client side) */

            /**
             * @brief The main loop of the Manager
             */
            void run();

            /**
             * @brief Get an available port for the client (only for client side)
             */
            std::uint16_t getAvailablePort();

            friend class Singleton<Manager>; /*!> Friend class to allow access to the private constructor and destructor */

            bool running_;                /*!> If the Manager is running */
            std::thread mainThread_;      /*!> The main thread of the Manager */
            connection::Side side_; /*!> The side of the connection (client or server) */

            std::shared_ptr<Tcp> tcp_; /*!> The tcp instance */
            std::thread tcpThread_;              /*!> The tcp thread */
            std::shared_ptr<Udp> udp_; /*!> The udp instance */
            std::thread udpThread_;              /*!> The udp thread */

            Callback callbacks_; /*!> The callback handler */

            std::queue<std::uint16_t> disconnectionQueue_; /*!> The queue of disconnections to process */
    };
}

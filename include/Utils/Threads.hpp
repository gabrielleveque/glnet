#pragma once

#include <thread>

namespace glnet::utils
{
    class Threads
    {
        public:
            /**
             * @brief Joins a thread if it's joinable
             *
             * @param thread The thread to join
             */
            static void join(std::thread& thread);
    };
}

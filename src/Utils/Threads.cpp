#include "Utils/Threads.hpp"

#include <iostream>

void glnet::utils::Threads::join(std::thread &thread)
{
    if (thread.joinable()) {
        thread.join();
    }
}

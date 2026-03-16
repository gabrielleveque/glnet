# glnet

## Overview
`glnet` is a lightweight and efficient network management library designed to simplify the creation and management of TCP and UDP servers and clients. It provides a robust and flexible API for handling network communication, making it suitable for a wide range of applications, from simple messaging systems to more complex multiplayer games or distributed systems.

## Features
- **TCP and UDP Support**: Easily create and manage both TCP and UDP servers and clients.
- **Modular Design**: The library is structured to allow easy extension and customization.
- **Thread Management**: Built-in utilities for managing threads in network operations.
- **Data Handling**: Includes utilities for handling packets and buffers.
- **Cross-Platform**: Compatible with multiple platforms, leveraging C++ standards.

## Project Structure
The project is organized as follows:

```
include/       # Header files for the library
src/           # Implementation files for the library
example/       # Example applications (client and server)
build/         # Build artifacts
```

### Key Directories
- **`include/`**: Contains all the public headers, such as `Callback.hpp`, `Manager.hpp`, and protocol-specific headers like `Tcp.hpp` and `Udp.hpp`.
- **`src/`**: Contains the implementation of the library, including utilities for data conversion, threading, and protocol handling.
- **`example/`**: Demonstrates how to use the library with example client and server applications.

## Getting Started

### Prerequisites
- A C++ compiler with C++17 support or higher.
- CMake (minimum version 3.10).

### Building the Project
1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd glnet
   ```
2. Create a build directory and configure the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```
3. Build the project:
   ```bash
   make
   ```

### Running the Examples
The `example/` directory contains sample client and server applications to demonstrate the usage of the library. (Build is to be implemented)

## Contributing
Contributions are welcome! If you have ideas for improvements or new features, feel free to open an issue or submit a pull request.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments
- Inspired by the need for simple and efficient network management tools.
- Thanks to the open-source community for providing valuable resources and tools.

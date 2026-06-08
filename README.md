# Bancho C++

An osu!Bancho server implementation in C++20 using the Crow framework.

## 📋 Description

**Bancho C++** is a simple server for the osu!, written in modern C++20. The project implements the Bancho protocol, which allows players to:

- Authenticate on the server
- Communicate via public and private messages

## 🛠️ Requirements

- **CMake** ≥ 3.20
- **C++20** compiler (GCC, Clang, MSVC)

## 📦 Dependencies

- **Crow** (v1.3.2) — modern C++ web framework

## 🚀 Build & Run

### Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The resulting executable will be in the `bin/` directory:
```bash
./bin/Debug/bancho
```

### Adding config

Create file in `/build/bin/Debug/` with name `config.json` or copy `config.json` into `/build/bin/Debug/`

### Running

After a successful build, the server will listen on port **18080** using HTTP:

```bash
./bin/Debug/bancho
```

The server is ready to accept requests from osu! clients.

## 📂 Project Structure

```
bancho.cpp/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
└── src/
    ├── main.cpp            # Entry point, server startup
    ├── cho.hpp             # Main Bancho request handler
    ├──
    ├── packets.hpp         # Protocol packet definitions (35+ types)
    └── player.hpp          # Player data structures and statuses
```
cmake_minimum_required(VERSION 3.16)

# I explicitly declare this as a C++ project to avoid CMake guessing.
project(WeatherApp LANGUAGES CXX)

# I standardize on C++17 to match Boost, OpenSSL, and modern STL usage.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All dependencies are provided through vcpkg (manifest mode).
# I use CONFIG mode for Boost to remain compatible with CMake 4.
find_package(Boost CONFIG REQUIRED COMPONENTS system)

# OpenSSL is needed for HTTPS requests in the WeatherClient.
find_package(OpenSSL REQUIRED)

# SQLite is used for lightweight local persistence.
find_package(SQLite3 REQUIRED)

# Header-only, but linked so CMake tracks include paths correctly.
find_package(nlohmann_json CONFIG REQUIRED)

# I list sources explicitly to keep the build predictable.
add_executable(WeatherApp
    src/main.cpp
    src/WeatherClient.cpp
    src/AuthService.cpp
    src/Database.cpp
    src/SessionManager.cpp
    src/HttpServer.cpp
    src/User.cpp
)

# I include src so internal headers can be included with quotes.
# External include paths are provided by imported targets.
target_include_directories(WeatherApp PRIVATE
    src
)

# I link only what the executable actually depends on.
target_link_libraries(WeatherApp PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
)

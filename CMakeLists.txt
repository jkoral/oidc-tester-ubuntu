cmake_minimum_required(VERSION 3.16)
project(oidc-tester VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/OIDCManager.cpp
    src/JWTDecoder.cpp
)

set(HEADERS
    src/MainWindow.h
    src/OIDCManager.h
    src/JWTDecoder.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)


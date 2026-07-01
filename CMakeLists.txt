cmake_minimum_required(VERSION 3.16)
project(nexus LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage(
        NAME sdbus-cpp
        GITHUB_REPOSITORY Kistler-Group/sdbus-cpp
        VERSION 2.3.1
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "SDBUSCPP_BUILD_TESTS OFF"
        "SDBUSCPP_BUILD_DOCS OFF"
        "SDBUSCPP_BUILD_EXAMPLES OFF"
)

# Define all source and header files
set(NEXUS_SOURCES
        src/main.cpp

        # Core App Actors
        src/app/p2p_group_owner.cpp
        src/app/p2p_client.cpp

        # Network Toolbox
        src/network/network_sandbox.cpp
        src/network/posix_link_configurator.cpp
        src/network/sdbus_p2p_controller.cpp

        # Utilities
        common/system_utils/system_utils.cpp
)

set(NEXUS_HEADERS
        # Core App Actors
        include/app/p2p_group_owner.h
        include/app/p2p_client.h

        # Network Toolbox
        include/network/network_sandbox.h
        include/network/posix_link_configurator.h
        include/network/sdbus_p2p_controller.h

        # Utilities
        common/system_utils/system_utils.h
        common/system_utils/scope_guard.h
        common/system_utils/UniqueFileLock.h
)

# Build the executable
add_executable(nexus ${NEXUS_SOURCES} ${NEXUS_HEADERS})

# Include directories so #include "app/..." works cleanly
target_include_directories(nexus PRIVATE include src common)

# Link dependencies
target_link_libraries(nexus PRIVATE sdbus-c++)
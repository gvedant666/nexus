# Install script for directory: /home/vedant/Code/nexus/build/_deps/sdbus-cpp-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sdbus-c++-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/libsdbus-c++.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sdbus-c++-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sdbus-c++" TYPE FILE FILES
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/Awaitable.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/ConvenienceApiClasses.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/ConvenienceApiClasses.inl"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/VTableItems.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/VTableItems.inl"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/Error.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/IConnection.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/AdaptorInterfaces.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/ProxyInterfaces.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/StandardInterfaces.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/IObject.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/IProxy.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/Message.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/MethodResult.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/Types.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/TypeTraits.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/Flags.h"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src/include/sdbus-c++/sdbus-c++.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sdbus-c++-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++/sdbus-c++-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++/sdbus-c++-targets.cmake"
         "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/CMakeFiles/Export/aae783ae287047a810449855b451586a/sdbus-c++-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++/sdbus-c++-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++/sdbus-c++-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++" TYPE FILE FILES "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/CMakeFiles/Export/aae783ae287047a810449855b451586a/sdbus-c++-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++" TYPE FILE FILES "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/CMakeFiles/Export/aae783ae287047a810449855b451586a/sdbus-c++-targets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sdbus-c++-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/sdbus-c++" TYPE FILE FILES
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/cmake/sdbus-c++-config.cmake"
    "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/cmake/sdbus-c++-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sdbus-c++-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build/pkgconfig/sdbus-c++.pc")
endif()


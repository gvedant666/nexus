# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src"
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-build"
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix"
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/tmp"
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp"
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src"
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

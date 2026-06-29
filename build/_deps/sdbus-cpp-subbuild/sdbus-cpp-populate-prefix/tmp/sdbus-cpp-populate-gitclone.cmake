# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if(EXISTS "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitclone-lastrun.txt" AND EXISTS "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitinfo.txt" AND
  "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitclone-lastrun.txt" IS_NEWER_THAN "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitinfo.txt")
  message(STATUS
    "Avoiding repeated git clone, stamp file is up to date: "
    "'/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitclone-lastrun.txt'"
  )
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/Kistler-Group/sdbus-cpp.git" "sdbus-cpp-src"
    WORKING_DIRECTORY "/home/vedant/Code/nexus/build/_deps"
    RESULT_VARIABLE error_code
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/Kistler-Group/sdbus-cpp.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"
          checkout "v2.3.1" --
  WORKING_DIRECTORY "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v2.3.1'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src"
    RESULT_VARIABLE error_code
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/home/vedant/Code/nexus/build/_deps/sdbus-cpp-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitinfo.txt" "/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/home/vedant/Code/nexus/build/_deps/sdbus-cpp-subbuild/sdbus-cpp-populate-prefix/src/sdbus-cpp-populate-stamp/sdbus-cpp-populate-gitclone-lastrun.txt'")
endif()

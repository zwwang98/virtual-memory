# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if(EXISTS "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitclone-lastrun.txt" AND EXISTS "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitinfo.txt" AND
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitclone-lastrun.txt" IS_NEWER_THAN "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitinfo.txt")
  message(STATUS
    "Avoiding repeated git clone, stamp file is up to date: "
    "'/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitclone-lastrun.txt'"
  )
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git" 
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/ThrowTheSwitch/Unity.git" "unity"
    WORKING_DIRECTORY "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity"
    RESULT_VARIABLE error_code
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/ThrowTheSwitch/Unity.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git" 
          checkout "v2.5.2" --
  WORKING_DIRECTORY "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v2.5.2'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity"
    RESULT_VARIABLE error_code
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitinfo.txt" "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/unity-gitclone-lastrun.txt'")
endif()

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity"
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-build"
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix"
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/tmp"
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp"
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src"
  "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/download/unity-prefix/src/unity-stamp${cfgdir}") # cfgdir has leading slash
endif()

cmake_minimum_required(VERSION 3.14)

project(ext/unity/download NONE)

include(ExternalProject)
ExternalProject_Add(unity
  GIT_REPOSITORY    https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG           v2.5.2
  SOURCE_DIR        "/Users/a9527/NEU/cs5600/VirtualFrameworkC/ext/unity/unity"
  BINARY_DIR        ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

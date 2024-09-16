cmake_minimum_required(VERSION 3.4)

project(oasis VERSION 0.0.1 DESCRIPTION "a collection of general-purpose c++ libraries")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")


# --- Configure Oasis ------------------------------------------------
add_library(oasis INTERFACE)
target_include_directories(oasis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(oasis
  PUBLIC
  FILE_SET oasis_headers TYPE HEADERS
  BASE_DIRS include
  FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/interaction/cli/parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/net/tcp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/os/kqueue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sync/channel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/time.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uuid.hpp
)

set_target_properties(oasis PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(oasis PROPERTIES SOVERSION 1)

# get_target_property(MYLIB_PUBLIC_HEADERS oasis INTERFACE_SOURCES)
# set_target_properties(oasis PROPERTIES PUBLIC_HEADER "${MYLIB_PUBLIC_HEADERS}")

install(TARGETS oasis
  FILE_SET oasis_headers
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/oasis"
)

# ------------------------------------------------------------------------------

# --- Fetch GoogleTest ---------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.13.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# ------------------------------------------------------------------------------

# --- Configure GoogleTest -----------------------------------------------------
enable_testing()

add_executable(
  oasis_test
  tst/channel_test.cpp
  tst/cli_test.cpp
  tst/kqueue_test.cpp
  tst/uuid_test.cpp
)
target_link_libraries(
  oasis_test
  GTest::gtest_main
  oasis
)

include(GoogleTest)
gtest_discover_tests(oasis_test)
# ------------------------------------------------------------------------------

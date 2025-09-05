cmake_minimum_required(VERSION 3.14)
project(OpenSSLTest)

# Include the OpenSSL integration
include(../CMakeLists.txt)

# Create a simple executable that links against OpenSSL
add_executable(openssl_test main.cpp)
target_link_libraries(openssl_test OpenSSL::SSL OpenSSL::Crypto)

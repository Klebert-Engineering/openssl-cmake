# FetchContent-Based OpenSSL CMake Integration
# Custom OpenSSL build configuration using ExternalProject

cmake_minimum_required(VERSION 3.14)

include(FetchContent)
include(ExternalProject)
include(ProcessorCount)

# Option to build shared libraries (defaults to static)
option(OPENSSL_SHARED "Build shared libraries instead of static" OFF)

# Option to show OpenSSL build output in real-time
option(OPENSSL_VERBOSE "Show OpenSSL build output in real-time" ON)

# Optional override: number of parallel jobs for OpenSSL build (Windows jom)
set(OPENSSL_JOBS "" CACHE STRING "Number of parallel jobs for OpenSSL build on Windows (jom). Defaults to detected core count.")

# Get number of processor cores for parallel builds (with fallbacks)
ProcessorCount(N)
if(N EQUAL 0 OR NOT N)
    if(WIN32 AND DEFINED ENV{NUMBER_OF_PROCESSORS})
        set(N $ENV{NUMBER_OF_PROCESSORS})
    else()
        cmake_host_system_information(RESULT N QUERY NUMBER_OF_LOGICAL_CORES)
        if(N STREQUAL "")
            set(N 1)
        endif()
    endif()
endif()

# Determine effective parallel jobs to use for OpenSSL build
if(OPENSSL_JOBS STREQUAL "")
    set(OPENSSL_JOBS_EFFECTIVE ${N})
else()
    set(OPENSSL_JOBS_EFFECTIVE ${OPENSSL_JOBS})
endif()

# Require OPENSSL_VERSION to be set - no fallback
if(NOT DEFINED OPENSSL_VERSION)
    message(FATAL_ERROR "OPENSSL_VERSION must be set. Example: -DOPENSSL_VERSION=openssl-3.5.1")
endif()

# Show build configuration
message(STATUS "OpenSSL Configuration:")
message(STATUS "  Version: ${OPENSSL_VERSION}")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  Library Type: ${OPENSSL_SHARED}")
message(STATUS "  Core Count: ${N}")
if(OPENSSL_SHARED)
    message(STATUS "  Building: Shared libraries")
else()
    message(STATUS "  Building: Static libraries")
endif()
if(OPENSSL_VERBOSE)
    message(STATUS "  Verbose Output: ON (real-time build output)")
else()
    message(STATUS "  Verbose Output: OFF (output logged to files)")
endif()

FetchContent_Declare(
    OpenSSL
    GIT_REPOSITORY https://github.com/openssl/openssl.git
    GIT_TAG        ${OPENSSL_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(OpenSSL)

# Set OpenSSL install directory
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/openssl-install)

# Use find_package so Perl executable can be set in cmake
find_package(Perl REQUIRED)

# Set shared/static configuration
if(OPENSSL_SHARED)
    set(OPENSSL_SHARED_FLAG "shared")
    set(OPENSSL_LIB_TYPE SHARED)
else()
    set(OPENSSL_SHARED_FLAG "no-shared")
    set(OPENSSL_LIB_TYPE STATIC)
endif()

# Determine platform-specific OpenSSL configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(OPENSSL_CONFIGURE_COMMAND ${PERL_EXECUTABLE} ${openssl_SOURCE_DIR}/Configure VC-WIN64A --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    else()
        set(OPENSSL_CONFIGURE_COMMAND ${PERL_EXECUTABLE} ${openssl_SOURCE_DIR}/Configure VC-WIN32 --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    endif()
    # Prefer jom for parallel builds; fallback to nmake
    find_program(JOM_EXECUTABLE NAMES jom)
    if(JOM_EXECUTABLE)
        set(OPENSSL_BUILD_COMMAND ${JOM_EXECUTABLE} -j${OPENSSL_JOBS_EFFECTIVE})
        set(OPENSSL_INSTALL_COMMAND ${JOM_EXECUTABLE} -j${OPENSSL_JOBS_EFFECTIVE} install_sw)
    else()
        set(OPENSSL_BUILD_COMMAND nmake)
        set(OPENSSL_INSTALL_COMMAND nmake install_sw)
    endif()
    # On Windows, OpenSSL may generate libraries with lib prefix
    set(OPENSSL_LIB_PREFIX "lib")
    if(OPENSSL_SHARED)
        set(OPENSSL_LIB_SUFFIX ".lib")
        set(OPENSSL_DLL_SUFFIX ".dll")
    else()
        set(OPENSSL_LIB_SUFFIX ".lib")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(OPENSSL_CONFIGURE_COMMAND ${openssl_SOURCE_DIR}/Configure darwin64-arm64-cc --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    else()
        set(OPENSSL_CONFIGURE_COMMAND ${openssl_SOURCE_DIR}/Configure darwin64-x86_64-cc --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    endif()
    if(NOT OPENSSL_JOBS_EFFECTIVE EQUAL 0)
        set(OPENSSL_BUILD_COMMAND make -j${OPENSSL_JOBS_EFFECTIVE})
        set(OPENSSL_INSTALL_COMMAND make -j${OPENSSL_JOBS_EFFECTIVE} install_sw)
    else()
        set(OPENSSL_BUILD_COMMAND make)
        set(OPENSSL_INSTALL_COMMAND make install_sw)
    endif()
    set(OPENSSL_LIB_PREFIX "lib")
    if(OPENSSL_SHARED)
        set(OPENSSL_LIB_SUFFIX ".dylib")
    else()
        set(OPENSSL_LIB_SUFFIX ".a")
    endif()
else() # Linux and other Unix-like systems
    # Detect architecture for Linux
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(OPENSSL_CONFIGURE_COMMAND ${openssl_SOURCE_DIR}/Configure linux-aarch64 --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(OPENSSL_CONFIGURE_COMMAND ${openssl_SOURCE_DIR}/Configure linux-x86_64 --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    else()
        set(OPENSSL_CONFIGURE_COMMAND ${openssl_SOURCE_DIR}/Configure linux-x86 --prefix=${OPENSSL_INSTALL_DIR} --openssldir=${OPENSSL_INSTALL_DIR}/ssl ${OPENSSL_SHARED_FLAG} no-tests)
    endif()
    if(NOT OPENSSL_JOBS_EFFECTIVE EQUAL 0)
        set(OPENSSL_BUILD_COMMAND make -j${OPENSSL_JOBS_EFFECTIVE})
        set(OPENSSL_INSTALL_COMMAND make -j${OPENSSL_JOBS_EFFECTIVE} install_sw)
    else()
        set(OPENSSL_BUILD_COMMAND make)
        set(OPENSSL_INSTALL_COMMAND make install_sw)
    endif()
    set(OPENSSL_LIB_PREFIX "lib")
    if(OPENSSL_SHARED)
        set(OPENSSL_LIB_SUFFIX ".so")
    else()
        set(OPENSSL_LIB_SUFFIX ".a")
    endif()
endif()

# Report parallelization details
message(STATUS "  Parallel jobs: ${OPENSSL_JOBS_EFFECTIVE}")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(JOM_EXECUTABLE)
        message(STATUS "  Windows make tool: jom (${JOM_EXECUTABLE})")
    else()
        message(STATUS "  Windows make tool: nmake (single-threaded). Install jom for -j${OPENSSL_JOBS_EFFECTIVE}.")
    endif()
endif()

# On MSVC, enable multi-processor compilation inside cl
if(MSVC)
    # Enable MSVC multi-proc and safe concurrent PDB writes
    set(OPENSSL_BUILD_COMMAND ${CMAKE_COMMAND} -E env "CL=/MP /FS" ${OPENSSL_BUILD_COMMAND})
    set(OPENSSL_INSTALL_COMMAND ${CMAKE_COMMAND} -E env "CL=/MP /FS" ${OPENSSL_INSTALL_COMMAND})
endif()

# Determine the library directory used by OpenSSL on Linux
# x86_64 Linux uses lib64, but ARM64 uses lib
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(OPENSSL_LIBDIR "lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OPENSSL_LIBDIR "lib64")
else()
    set(OPENSSL_LIBDIR "lib")
endif()

# Build OpenSSL using ExternalProject
ExternalProject_Add(openssl_build
    SOURCE_DIR ${openssl_SOURCE_DIR}
    CONFIGURE_COMMAND ${OPENSSL_CONFIGURE_COMMAND}
    BUILD_COMMAND ${OPENSSL_BUILD_COMMAND}
    INSTALL_COMMAND ${OPENSSL_INSTALL_COMMAND}
    BUILD_IN_SOURCE 1
    BUILD_BYPRODUCTS 
        # Windows libraries (both static and shared use .lib for linking)
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>:${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX}>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>:${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX}>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>:${OPENSSL_INSTALL_DIR}/lib/ssl${OPENSSL_LIB_SUFFIX}>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>:${OPENSSL_INSTALL_DIR}/lib/crypto${OPENSSL_LIB_SUFFIX}>
        # Windows DLLs (for shared builds)
        $<$<AND:$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>,$<BOOL:${OPENSSL_SHARED}>>:${OPENSSL_INSTALL_DIR}/bin/${OPENSSL_LIB_PREFIX}ssl-3-x64${OPENSSL_DLL_SUFFIX}>
        $<$<AND:$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>,$<BOOL:${OPENSSL_SHARED}>>:${OPENSSL_INSTALL_DIR}/bin/${OPENSSL_LIB_PREFIX}crypto-3-x64${OPENSSL_DLL_SUFFIX}>
        # Linux libraries
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:${OPENSSL_INSTALL_DIR}/${OPENSSL_LIBDIR}/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX}>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:${OPENSSL_INSTALL_DIR}/${OPENSSL_LIBDIR}/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX}>
        $<$<AND:$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>,$<NOT:$<STREQUAL:${OPENSSL_LIBDIR},lib>>>:${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX}>
        $<$<AND:$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>,$<NOT:$<STREQUAL:${OPENSSL_LIBDIR},lib>>>:${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX}>
        # macOS libraries
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>:${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX}>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>:${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX}>
    LOG_CONFIGURE $<IF:$<BOOL:${OPENSSL_VERBOSE}>,OFF,ON>
    LOG_BUILD $<IF:$<BOOL:${OPENSSL_VERBOSE}>,OFF,ON>
    LOG_INSTALL $<IF:$<BOOL:${OPENSSL_VERBOSE}>,OFF,ON>
    LOG_OUTPUT_ON_FAILURE $<IF:$<BOOL:${OPENSSL_VERBOSE}>,OFF,ON>
    # When OPENSSL_VERBOSE is ON, output goes directly to console
    USES_TERMINAL_CONFIGURE $<BOOL:${OPENSSL_VERBOSE}>
    USES_TERMINAL_BUILD $<BOOL:${OPENSSL_VERBOSE}>
    USES_TERMINAL_INSTALL $<BOOL:${OPENSSL_VERBOSE}>
)

# Create imported targets for OpenSSL::SSL and OpenSSL::Crypto
add_library(OpenSSL::SSL ${OPENSSL_LIB_TYPE} IMPORTED GLOBAL)
add_library(OpenSSL::Crypto ${OPENSSL_LIB_TYPE} IMPORTED GLOBAL)

# Create the include and lib directories early to avoid CMake configuration errors
file(MAKE_DIRECTORY ${OPENSSL_INSTALL_DIR}/include)
file(MAKE_DIRECTORY ${OPENSSL_INSTALL_DIR}/lib)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    file(MAKE_DIRECTORY ${OPENSSL_INSTALL_DIR}/lib64)
endif()
# Create bin directory for Windows DLLs
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND OPENSSL_SHARED)
    file(MAKE_DIRECTORY ${OPENSSL_INSTALL_DIR}/bin)
endif()

# Set target properties - use standard lib path
# On Windows, handle both static and shared libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(OPENSSL_SHARED)
        # For shared libraries on Windows, we need both the DLL and the import library
        set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/bin/libssl-3-x64${OPENSSL_DLL_SUFFIX}
            IMPORTED_IMPLIB ${OPENSSL_INSTALL_DIR}/lib/ssl${OPENSSL_LIB_SUFFIX}
            INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
        )
        set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/bin/libcrypto-3-x64${OPENSSL_DLL_SUFFIX}
            IMPORTED_IMPLIB ${OPENSSL_INSTALL_DIR}/lib/crypto${OPENSSL_LIB_SUFFIX}
            INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
        )
    else()
        # Static libraries
        set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/ssl${OPENSSL_LIB_SUFFIX}
            INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
        )
        set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/crypto${OPENSSL_LIB_SUFFIX}
            INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
        )
    endif()
else()
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX}
        INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
    )
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX}
        INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
    )
endif()

# Add a post-build command to handle lib64 vs lib directory issue on Linux x86_64 systems only
# This ensures libraries are available in the expected lib/ directory
# macOS and ARM64 Linux install directly to lib/, so this is only needed for x86_64 Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    add_custom_command(
        TARGET openssl_build POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OPENSSL_INSTALL_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${OPENSSL_INSTALL_DIR}/lib64/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX} 
            ${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${OPENSSL_INSTALL_DIR}/lib64/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX} 
            ${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX}
        COMMENT "Ensuring OpenSSL libraries are available in lib/ directory"
        VERBATIM
    )
endif()

# Add a post-build command to handle library naming issues on Windows
# OpenSSL may generate libraries with different names, so we create fallback copies
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(
        TARGET openssl_build POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}ssl${OPENSSL_LIB_SUFFIX} 
            ${OPENSSL_INSTALL_DIR}/lib/ssl${OPENSSL_LIB_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${OPENSSL_INSTALL_DIR}/lib/${OPENSSL_LIB_PREFIX}crypto${OPENSSL_LIB_SUFFIX} 
            ${OPENSSL_INSTALL_DIR}/lib/crypto${OPENSSL_LIB_SUFFIX}
        COMMENT "Ensuring OpenSSL libraries are available with expected names"
        VERBATIM
    )
endif()

# Add dependencies
add_dependencies(OpenSSL::SSL openssl_build)
add_dependencies(OpenSSL::Crypto openssl_build)

# Link SSL to Crypto
set_target_properties(OpenSSL::SSL PROPERTIES
    INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
)

# Platform-specific system libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(OpenSSL::Crypto PROPERTIES
        INTERFACE_LINK_LIBRARIES "ws2_32;gdi32;advapi32;crypt32;user32"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(OpenSSL::Crypto PROPERTIES
        INTERFACE_LINK_LIBRARIES "dl;pthread"
    )
endif() 
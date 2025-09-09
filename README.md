# OpenSSL CMake FetchContent Integration

A cross-platform CMake module for building OpenSSL from source using FetchContent and ExternalProject.

## Features

- **Cross-platform support**: Windows, macOS, and Linux (x86_64 and ARM64)
- **Automatic dependency management**: Uses CMake's FetchContent to download OpenSSL source
- **Static and shared library builds**: Configurable via `OPENSSL_SHARED` option
- **Parallel builds**: Automatically detects CPU cores for faster compilation
- **Standard CMake targets**: Provides `OpenSSL::SSL` and `OpenSSL::Crypto` imported targets
- **CI/CD tested**: GitHub Actions workflow tests builds across all supported platforms

## Usage

### Basic Usage

```cmake
# REQUIRED: Set the OpenSSL version
set(OPENSSL_VERSION "openssl-3.5.1")  # or any other version tag

# Include the CMake module
include(path/to/CMakeLists.txt)

# Your targets can now link against OpenSSL
target_link_libraries(your_target OpenSSL::SSL OpenSSL::Crypto)
```

### Configuration Options

#### OpenSSL Version (Required)

The `OPENSSL_VERSION` variable must be set before including the CMakeLists.txt. There is no default fallback:

```cmake
set(OPENSSL_VERSION "openssl-3.4.0")  # Required - no default
include(path/to/CMakeLists.txt)
```

#### Static vs Shared Libraries

Control whether to build static or shared libraries using the `OPENSSL_SHARED` option:

```cmake
set(OPENSSL_SHARED OFF)  # Default: OFF (static libraries)
# or
set(OPENSSL_SHARED ON)   # Build shared libraries (.dll/.so/.dylib)

set(OPENSSL_VERSION "openssl-3.5.1")
include(path/to/CMakeLists.txt)
```

## Requirements

- CMake 3.14+
- Perl (required for OpenSSL configuration)
- NASM (Netwide Assembler) - required for OpenSSL builds
- Platform-specific build tools:
  - **Windows**: Visual Studio with nmake
  - **macOS**: Xcode Command Line Tools
  - **Linux**: GCC/Clang and make

## Notes

- The `OPENSSL_VERSION` variable is required and must be set before including the CMake module
- Default build type is static libraries. Use `OPENSSL_SHARED=ON` for shared libraries
- On Windows, shared builds will produce both .lib (import libraries) and .dll files

## License

BSD 3-Clause License - see [LICENSE](LICENSE) file for details.

# OpenSSL CMake FetchContent Integration

A cross-platform CMake module for building OpenSSL from source using FetchContent and ExternalProject.

## Features

- **Cross-platform support**: Windows, macOS, and Linux (x86_64 and ARM64)
- **Automatic dependency management**: Uses CMake's FetchContent to download OpenSSL source
- **Static library builds**: Configures OpenSSL for static linking with `no-shared`
- **Parallel builds**: Automatically detects CPU cores for faster compilation
- **Standard CMake targets**: Provides `OpenSSL::SSL` and `OpenSSL::Crypto` imported targets
- **CI/CD tested**: GitHub Actions workflow tests builds across all supported platforms

## Usage

Simply include this CMakeLists.txt in your project, or just add it via `add_subdirectory`:

```cmake
include(path/to/CMakeLists.txt)

# Your targets can now link against OpenSSL
target_link_libraries(your_target OpenSSL::SSL OpenSSL::Crypto)
```

### Custom OpenSSL Version

To use a different OpenSSL version, set the `OPENSSL_VERSION` variable before including the CMakeLists.txt:

```cmake
set(OPENSSL_VERSION "openssl-3.4.0")  # or any other version tag
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

## OpenSSL Version

Default version is OpenSSL 3.5.1. Use the `OPENSSL_VERSION` variable to override (see Usage section above).

## License

BSD 3-Clause License - see [LICENSE](LICENSE) file for details.

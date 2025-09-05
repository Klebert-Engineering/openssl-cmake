#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <iostream>

int main() {
    std::cout << "OpenSSL version: " << OpenSSL_version(OPENSSL_VERSION) << std::endl;
    std::cout << "SSL version: " << OpenSSL_version(SSLEAY_VERSION) << std::endl;
    std::cout << "Crypto version: " << OpenSSL_version(SSLEAY_VERSION) << std::endl;
    
    // Test basic SSL functionality
    SSL_library_init();
    SSL_load_error_strings();
    
    std::cout << "OpenSSL integration test successful!" << std::endl;
    return 0;
}

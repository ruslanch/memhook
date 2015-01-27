#include <memhook/common.hpp>
#include <memhook/network.hpp>

int main(int argc, char const *argv[])
{
    try {
        asio::io_service io_service;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

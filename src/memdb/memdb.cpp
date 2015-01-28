#include <memhook/common.hpp>
#include <memhook/network.hpp>
#include "memdb_server.hpp"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <iostream>
#include <cstdlib>

namespace memhook { namespace detail {
    namespace po = program_options;

    void usage(const po::options_description &options) {
        std::cout << "Usage: memdb [options] [-m | -f path] [-p PORT_NUM]\n";
        std::cout << options << std::endl;
    }

    bool parse_command_line_arguments(int argc, char const *argv[], po::variables_map &map) {
        po::options_description options("Options");
        options.add_options()
            ("help",  "show help message")
            ("shared-memory,m",   po::value<std::string>()->implicit_value(MEMHOOK_SHARED_MEMORY),
                "use shared memory")
            ("mapped-file,f",     po::value<std::string>(),
                "use memory mapped file")
            ("address,a",            po::value<std::string>()->default_value("127.0.0.1"),
                "address")
            ("service,s",            po::value<std::string>()->default_value(
                    BOOST_PP_STRINGIZE(MEMHOOK_NETWORK_STORAGE_PORT)),
                "service");

        try {
            po::positional_options_description positional_options;
            positional_options.add("mapped-file", -1);
            program_options::store(program_options::command_line_parser(argc, argv)
                .options(options)
                .positional(positional_options)
                .run(), map);
            program_options::notify(map);

            if (map.count("help") || !map.count("address") || !map.count("service")) {
                usage(options);
                return false;
            }
        }
        catch (const po::error &e) {
            std::cerr << e.what() << std::endl;
            return false;
        }

        return true;
    }
}} // memhook::detail

int main(int argc, char const *argv[])
{
    using namespace memhook;
    using namespace memhook::detail;

    try {
        po::variables_map options_map;
        if (!parse_command_line_arguments(argc, argv, options_map))
            return EXIT_FAILURE;

        asio::io_service io_service;
        asio::signal_set signals(io_service, SIGINT, SIGTERM, SIGQUIT);
        signals.async_wait(bind(&asio::io_service::stop, &io_service));

        memdb_server server(io_service,
                options_map["address"].as<std::string>(),
                options_map["service"].as<std::string>());
        io_service.run();

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

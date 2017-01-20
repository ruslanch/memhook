#include "common.h"
#include "memdb_server.h"

#include <memhook/network.h>

#include <boost/asio.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <cstdlib>
#include <iostream>

namespace memhook {
  namespace detail {
    namespace po = boost::program_options;

    void usage(const po::options_description &options) {
      std::cout << "Usage: MemDB [options] [-m | -f path] [-p PORT_NUM]\n";
      std::cout << options << std::endl;
    }

    bool parse_command_line_arguments(int argc, char const *argv[], po::variables_map &map) {
      const std::size_t default_storage_size =
#ifdef __x86_64__
            (8ul << 30)
#else
            (1ul << 30)
#endif
            ;

      po::options_description options("Options");
      options.add_options()("help", "show help message")("shared-memory,m",
              po::value<std::string>()->implicit_value(MEMHOOK_SHARED_MEMORY),
              "use shared memory")(
              "mapped-file,f", po::value<std::string>(), "use memory mapped file")("size,s",
              po::value<std::size_t>()->default_value(default_storage_size),
              "size")("host,h", po::value<std::string>()->default_value("127.0.0.1"), "host")(
              "port,p", po::value<int>()->default_value(MEMHOOK_NETWORK_STORAGE_PORT), "port");

      try {
        po::positional_options_description positional_options;
        positional_options.add("mapped-file", -1);
        po::store(po::command_line_parser(argc, argv)
                          .options(options)
                          .positional(positional_options)
                          .run(),
                map);
        po::notify(map);

        if (map.count("shared-memory") && map.count("mapped-file")) {
          std::cerr << "can't use `--shared-memory` and `--mapped-file` "
                       "options simultaneously"
                    << std::endl;
          return false;
        }

        if (map.count("help") || !map.count("host") || !map.count("port")) {
          usage(options);
          return false;
        }
      } catch (const po::error &e) {
        std::cerr << e.what() << std::endl;
        return false;
      }

      return true;
    }
  }
}  // memhook::detail

int main(int argc, char const *argv[]) {
  using namespace memhook;
  using namespace memhook::detail;

  try {
    po::variables_map options_map;
    if (!parse_command_line_arguments(argc, argv, options_map))
      return EXIT_FAILURE;

    shared_ptr<MappedStorageCreator> storage_creator;
    if (options_map.count("mapped-file")) {
      std::string file_path = options_map["mapped-file"].as<std::string>();
      unique_ptr<MappedStorageCreator> tmp(
              NewMMFMappedStorageCreator(file_path.c_str(), options_map["size"].as<std::size_t>()));
      storage_creator.reset(tmp.release());
    } else {
      std::string name;
      if (options_map.count("shared-memory"))
        name = options_map["shared-memory"].as<std::string>();
      else
        name = MEMHOOK_SHARED_MEMORY;
      unique_ptr<MappedStorageCreator> tmp(
              NewSHMMappedStorageCreator(name.c_str(), options_map["size"].as<std::size_t>()));
      storage_creator.reset(tmp.release());
    }

    boost::asio::io_service io_service;
    boost::asio::signal_set signals(io_service, SIGINT, SIGTERM, SIGQUIT);
    signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

    MemDBServer server(storage_creator,
            io_service,
            options_map["host"].as<std::string>().c_str(),
            options_map["port"].as<int>());

    boost::thread_group tg;
    const unsigned hardware_concurrency = boost::thread::hardware_concurrency();
    for (unsigned i = 0; i < hardware_concurrency; ++i)
      tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    tg.join_all();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

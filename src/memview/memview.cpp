#include "common.h"
#include "mapped_view.h"

#include <memhook/chrono_utils.h>

#include <boost/checked_delete.hpp>
#include <boost/foreach.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/tuple/tuple.hpp>
#include <cstdlib>
#include <iostream>

namespace memhook {
  namespace detail {
    namespace po = boost::program_options;

    void ShowUsage(const po::options_description &options) {
      std::cout << "Usage: memview [options] [-m | -f path]\n";
      std::cout << options << std::endl;
    }

    bool parse_command_line_arguments(int argc, char const *argv[], po::variables_map &map) {
      po::options_description options("Options");
      options.add_options()
              ("help",
                      "show help message")
              ("shared-memory,m",   po::value<std::string>()->implicit_value(MEMHOOK_SHARED_MEMORY),
                      "use shared memory")
              ("mapped-file,f",     po::value<std::string>(),
                      "use memory mapped file")
              ("show-callstack,c",  po::value<bool>()->implicit_value(true),
                      "print callstack")
              ("aggregate,a",       po::value<bool>()->implicit_value(true),
                      "aggregate")
              ("sort-by-time,t",
                      "sort by timestamp")
              ("sort-by-size,s",
                      "sort by allocation size")
              ("sort-by-address,p",
                      "sort by allocation address")
              ("min-time-from-now", po::value<std::string>(),
                      "minimal time interval from current time to allocation time (10s, 30min, 5h)")
              ("max-time-from-now", po::value<std::string>(),
                      "maximum time interval from current time to allocation time (10s, 30min, 5h)")
              ("min-time-from-start", po::value<std::string>(),
                      "minimal time interval from program start to allocation time (10s, 30min, 5h)")
              ("max-time-from-start", po::value<std::string>(),
                      "maximum time interval from program start to allocation time (10s, 30min, 5h)")
              ("min-time,n",        po::value<std::string>(),
                      "minimal allocation datetime (YYYY-MM-DD HH:MM:SS)")
              ("max-time,x",        po::value<std::string>(),
                      "maximum allocation datetime (YYYY-MM-DD HH:MM:SS)")
              ("min-size,z",        po::value<std::size_t>(),
                      "minimal allocation size (in bytes)")
              ("get-storage-info",
                      "")
              ("no-lock", po::value<bool>()->implicit_value(true),
                      "");

      try {
        po::positional_options_description positional_options;
        positional_options.add("mapped-file", -1);
        po::store(po::command_line_parser(argc, argv)
                          .options(options)
                          .positional(positional_options)
                          .run(),
                map);
        po::notify(map);

        if (map.count("help")) {
          ShowUsage(options);
          return false;
        }

        if (map.count("shared-memory") && map.count("mapped-file")) {
          std::cerr << "can't use `--shared-memory` and `--mapped-file` "
                    << "options simultaneously"
                    << std::endl;
          return false;
        }
      } catch (const po::error &e) {
        std::cerr << e.what() << std::endl;
        return false;
      }

      return true;
    }

    chrono::system_clock::time_point ChronoTimePointFromString(const std::string &string) {
      chrono::system_clock::time_point time_point;
      memhook::ChronoTimePointFromString(string, time_point);
      return time_point;
    }

    chrono::system_clock::duration ChronoDurationFromString(const std::string &string) {
      chrono::system_clock::duration duration;
      memhook::ChronoDurationFromString(string, duration);
      return duration;
    }
  }
}

int main(int argc, char const *argv[]) {
  std::ios::sync_with_stdio(false);

  using namespace memhook;
  using namespace memhook::detail;

  po::variables_map options_map;
  if (!parse_command_line_arguments(argc, argv, options_map))
    return EXIT_FAILURE;

  try {
    std::string path;
    unique_ptr<MappedViewWorks> works;

    if (options_map.count("mapped-file")) {
      path = options_map["mapped-file"].as<std::string>();
      works = NewMMFMappedViewWorks();
    } else {
      if (options_map.count("shared-memory"))
        path = options_map["shared-memory"].as<std::string>();
      else
        path = MEMHOOK_SHARED_MEMORY;
      works = NewSHMMappedViewWorks();
    }

    unique_ptr<MappedView> view;
    if (options_map.count("aggregate")) {
      view = works->NewAggregatedView(path.c_str());
    } else {
      view = works->NewSimpleView(path.c_str());
    }

    if (options_map.count("no-lock")) {
      view->SetOptionFlag(MappedView::kNoLock, options_map["no-lock"].as<bool>());
    }

    if (options_map.count("get-storage-info")) {
      std::cout << "size=" << view->GetSize() << ", free=" << view->GetFreeMemory() << std::endl;
      return EXIT_SUCCESS;
    }

    BOOST_FOREACH (const po::variables_map::value_type &op, options_map) {
      if (op.first == "sort-by-address") {
        view->SetOptionFlag(MappedView::kSortByAddress, true);
      } else if (op.first == "sort-by-size") {
        view->SetOptionFlag(MappedView::kSortBySize, true);
      } else if (op.first == "sort-by-time") {
        view->SetOptionFlag(MappedView::kSortByTime, true);
      } else if (op.first == "show-callstack") {
        view->SetOptionFlag(MappedView::kShowCallStack, op.second.as<bool>());
      } else if (op.first == "min-time-from-now") {
        view->SetOption(works->NewMinTimeFromNowOption(
                ChronoDurationFromString(op.second.as<std::string>())));
      } else if (op.first == "max-time-from-now") {
        view->SetOption(works->NewMaxTimeFromNowOption(
                ChronoDurationFromString(op.second.as<std::string>())));
      } else if (op.first == "min-time-from-start") {
        view->SetOption(works->NewMinTimeFromStartOption(
                ChronoDurationFromString(op.second.as<std::string>())));
      } else if (op.first == "max-time-from-start") {
        view->SetOption(works->NewMaxTimeFromStartOption(
                ChronoDurationFromString(op.second.as<std::string>())));
      } else if (op.first == "min-time") {
        view->SetOption(works->NewMinTimeOption(
                ChronoTimePointFromString(op.second.as<std::string>())));
      } else if (op.first == "max-time") {
        view->SetOption(works->NewMaxTimeOption(
                ChronoTimePointFromString(op.second.as<std::string>())));
      } else if (op.first == "min-size") {
        view->SetOption(works->NewMinSizeOption(op.second.as<std::size_t>()));
      }
    }

    view->Write(std::cout);

  } catch (const boost::interprocess::interprocess_exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

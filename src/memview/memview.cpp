#include <memhook/common.hpp>
#include "mapped_view.hpp"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/checked_delete.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <iostream>
#include <cstdlib>

namespace memhook { namespace detail {
    namespace po = program_options;

    void usage(const po::options_description &options) {
        std::cout << "Usage: memtraceview [options] [-m | -f path]\n";
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
            ("show-callstack,c",  po::value<bool>()->implicit_value(true), "print callstack")
            ("sort-by-time,t",    "sort by timestamp")
            ("sort-by-size,s",    "sort by allocation size")
            ("sort-by-address,p", "sort by allocation address")
            ("min-time-from-now", po::value<std::string>(),
                "minimal time interval from current time to allocation time (10 sec, 30 min, 5 hours)")
            ("max-time-from-now", po::value<std::string>(),
                "maximum time interval from current time to allocation time (10 sec, 30 min, 5 hours)")
            ("min-time,n",        po::value<std::string>(),
                "minimal allocation datetime (YYYY-MM-DD HH:MM:SS)")
            ("max-time,x",        po::value<std::string>(),
                "maximum allocation datetime (YYYY-MM-DD HH:MM:SS)")
            ("min-size,z",        po::value<std::size_t>(), "minimal allocation size (in bytes)")
            ("get-storage-info", "")
            ("no-lock", po::value<bool>()->implicit_value(true), "");

        try {
            po::positional_options_description positional_options;
            positional_options.add("path", -1);
            program_options::store(program_options::command_line_parser(argc, argv)
                .options(options)
                .positional(positional_options)
                .run(), map);
            program_options::notify(map);

            if (map.count("help")) {
                usage(options);
                return false;
            }

            if (map.count("shared-memory") && map.count("mapped-file")) {
                std::cout << "can't use `--shared-memory` and `--mapped-file` "
                    "options simultaneously" << std::endl;
                return false;
            }
        }
        catch (const po::error &e) {
            std::cout << e.what() << std::endl;
            return false;
        }

        return true;
    }

    system_clock_t::time_point time_point_from_string(const std::string &string) {
        std::istringstream sstream(string);
        system_clock_t::time_point time_point;
        sstream >> chrono::time_fmt(chrono::timezone::local, "%Y-%m-%d %H:%M:%S") >> time_point;
        return time_point;
    }

    system_clock_t::duration duration_from_string(const std::string &string) {
        std::istringstream sstream(string);
        system_clock_t::duration duration;
        sstream >> duration;
        return duration;
    }

}} // memhook.detail

int main(int argc, char const *argv[])
{
    std::ios::sync_with_stdio(false);

    // using namespace boost_private;
    // using namespace boost;
    using namespace memhook;
    using namespace memhook::detail;

    po::variables_map options_map;
    if (!parse_command_line_arguments(argc, argv, options_map))
        return EXIT_FAILURE;

    try
    {
        tuple<std::string, movelib::unique_ptr<mapped_view_kit> > ctx;
        if (options_map.count("mapped-file"))
        {
            get<0>(ctx) = options_map["mapped-file"].as<std::string>();
            movelib::unique_ptr<mapped_view_kit> kit(make_mmf_view_kit());
            get<1>(ctx).swap(kit);
        }
        else
        {
            if (options_map.count("shared-memory"))
                get<0>(ctx) = options_map["shared-memory"].as<std::string>();
            else
                get<0>(ctx) = MEMHOOK_SHARED_MEMORY;
            movelib::unique_ptr<mapped_view_kit> kit(make_shm_view_kit());
            get<1>(ctx).swap(kit);
        }

        movelib::unique_ptr<mapped_view> view(get<1>(ctx)->make_view(get<0>(ctx).c_str()));
        if (options_map.count("no-lock"))
            view->set_option(mapped_view::no_lock, options_map["no-lock"].as<bool>());

        if (options_map.count("get-storage-info")) {
            std::cout << "size="   << view->get_size()
                      << ", free=" << view->get_free_memory() << std::endl;
            return EXIT_SUCCESS;
        }

        BOOST_FOREACH(const po::variables_map::value_type &op, options_map)
        {
            if (op.first == "sort-by-address")
                view->set_option(mapped_view::sort_by_address);
            else if (op.first == "sort-by-size")
                view->set_option(mapped_view::sort_by_size);
            else if (op.first == "sort-by-time")
                view->set_option(mapped_view::sort_by_time);

            if (op.first == "show-callstack")
                view->set_option(mapped_view::show_callstack, op.second.as<bool>());

            if (op.first == "min-time-from-now")
                view->add_req(get<1>(ctx)->make_min_time_from_now_req(duration_from_string(
                    op.second.as<std::string>())));
            if (op.first == "max-time-from-now")
                view->add_req(get<1>(ctx)->make_max_time_from_now_req(duration_from_string(
                    op.second.as<std::string>())));
            if (op.first == "min-time")
                view->add_req(get<1>(ctx)->make_min_time_req(time_point_from_string(
                    op.second.as<std::string>())));
            if (op.first == "max-time")
                view->add_req(get<1>(ctx)->make_max_time_req(time_point_from_string(
                    op.second.as<std::string>())));
            if (op.first == "min-size")
                view->add_req(get<1>(ctx)->make_min_size_req(
                    op.second.as<std::size_t>()));
        }
        view->write(std::cout);
    }
    catch (const interprocess::interprocess_exception &e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

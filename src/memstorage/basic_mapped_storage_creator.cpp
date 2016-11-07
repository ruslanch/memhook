#include "basic_mapped_storage_creator.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <sys/types.h>
#include <unistd.h>

namespace memhook
{

std::string BasicMappedStorageCreatorMixin::GenerateUniquePath(
        const std::string &original_path, uintptr_t guide) const
{
    using namespace boost::filesystem;

    const path orig_path(original_path);
    if (!boost::filesystem::exists(orig_path))
        return original_path;

    const std::string extension = orig_path.extension().string();
    const std::string pid_extension = "." + boost::lexical_cast<std::string>(guide ? guide : ::getpid());
    const std::string unique_path_base = (orig_path.parent_path() / orig_path.stem()).string() + pid_extension;

    boost::filesystem::path unique_path;
    std::string num_extension;
    for (size_t i = 0; ; ++i)
    {
        unique_path = unique_path_base + num_extension + extension;
        if (!boost::filesystem::exists(unique_path))
            break;

        num_extension = "." + boost::lexical_cast<std::string>(i);
    }
    return unique_path.string();
}

} // memhook

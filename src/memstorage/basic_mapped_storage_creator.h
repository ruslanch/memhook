#ifndef MEMHOOK_SRC_MEMSTORAGE_BASIC_MAPPED_STORAGE_CREATOR_H_INCLUDED
#define MEMHOOK_SRC_MEMSTORAGE_BASIC_MAPPED_STORAGE_CREATOR_H_INCLUDED

#include "common.h"
#include <memhook/mapped_storage_creator.h>

namespace memhook
{

class BasicMappedStorageCreatorMixin
{
protected:
    BasicMappedStorageCreatorMixin() {};

    std::string GenerateUniquePath(const std::string &original_path, uintptr_t guide) const;
};

template <typename MappedStorageT>
class BasicMappedStorageCreator : public MappedStorageCreator, BasicMappedStorageCreatorMixin
{
public:
    BasicMappedStorageCreator(const char *path, std::size_t size)
        : path_(path), size_(size) {}

    unique_ptr<MappedStorage> New(uintptr_t guide) const
    {
        return unique_ptr<MappedStorage>(new MappedStorageT(GenerateUniquePath(path_, guide).c_str(), size_));
    }

private:
    std::string path_;
    std::size_t size_;
};

} // memhook

#endif

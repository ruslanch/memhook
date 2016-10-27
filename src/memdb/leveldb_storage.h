#ifndef MEMHOOK_SRC_MEMDB_LEVELDB_STORAGE_H_INCLUDED
#define MEMHOOK_SRC_MEMDB_LEVELDB_STORAGE_H_INCLUDED

#include "common.h"

#include <memhook/mapped_storage.h>

#include <boost/filesystem/path.hpp>

#include <leveldb/db.h>
#include <leveldb/cache.h>

namespace memhook
{

class LevelDBStorage : public MappedStorage
{
public:
    LevelDBStorage(const std::string &path, std::size_t cache_size_mb = 8);

    void Insert(uintptr_t address, std::size_t memsize,
        const boost::chrono::system_clock::time_point &timestamp,
        const CallStackInfo &callstack);
    bool Erase(uintptr_t address);
    bool UpdateSize(uintptr_t address, std::size_t memsize);
    void Clear();

private:
    unique_ptr<leveldb::DB> OpenDB(const leveldb::Options &options,
            const boost::filesystem::path &path) const;

    boost::filesystem::path path_;
    std::size_t cache_size_mb_;

    unique_ptr<leveldb::Cache> dcache_;
    unique_ptr<leveldb::DB>    dattab_;
    unique_ptr<leveldb::DB>    symtab_;
    unique_ptr<leveldb::DB>    shltab_;
};

} // memhook

#endif

#ifndef MEMHOOK_SRC_MEMDB_LEVELDB_STORAGE_H_INCLUDED
#define MEMHOOK_SRC_MEMDB_LEVELDB_STORAGE_H_INCLUDED

#include "common.h"

#include <memhook/mapped_storage.h>

#include <boost/filesystem/path.hpp>

#include <leveldb/db.h>
#include <leveldb/cache.h>

namespace memhook {
  class LevelDBStorage : public MappedStorage {
  public:
    LevelDBStorage(const char *path, std::size_t cache_size_mb = 8);

    void Add(uintptr_t address, std::size_t memsize, const CallStackInfo &callstack,
            const chrono::system_clock::time_point &timestamp);
    bool Remove(uintptr_t address);
    bool UpdateSize(uintptr_t address, std::size_t memsize);
    void Clear();
    void Flush();
    std::string GetName() const;

  private:
    unique_ptr<leveldb::DB> OpenDB(const leveldb::Options &options,
            const boost::filesystem::path &path) const;

    boost::filesystem::path m_path;
    std::size_t    m_cache_size_mb;

    unique_ptr<leveldb::Cache> m_dcache;
    unique_ptr<leveldb::DB>    m_dattab;
    unique_ptr<leveldb::DB>    m_symtab;
    unique_ptr<leveldb::DB>    m_shltab;
  };
}

#endif

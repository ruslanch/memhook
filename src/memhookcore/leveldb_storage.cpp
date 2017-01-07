#include "leveldb_storage.h"

#include <boost/chrono/system_clocks.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/throw_exception.hpp>

#include <leveldb/comparator.h>

namespace memhook {
  namespace {
    class SystemClockComparator : public leveldb::Comparator {
    public:
      int Compare(const leveldb::Slice &lhs, const leveldb::Slice &rhs) const {
        const chrono::system_clock::rep lhs_rep =
                *reinterpret_cast<const chrono::system_clock::rep *>(lhs.data());

        const chrono::system_clock::rep rhs_rep =
                *reinterpret_cast<const chrono::system_clock::rep *>(rhs.data());

        if (lhs_rep < rhs_rep)
          return -1;

        if (lhs_rep > rhs_rep)
          return 1;

        return 0;
      }

      const char *Name() const {
        return "SystemClockComparator";
      }
      void FindShortestSeparator(std::string *, const leveldb::Slice &) const {}
      void FindShortSuccessor(std::string *) const {}
    };

    SystemClockComparator SYSTEM_CLOCK_COMPARATOR;
  }  // namespace

  LevelDBStorage::LevelDBStorage(const char *path, std::size_t cache_size_mb)
      : m_path(path)
      , m_cache_size_mb(cache_size_mb) {
    if (!boost::filesystem::is_directory(m_path)) {
      if (boost::filesystem::exists(m_path))
        boost::filesystem::remove(m_path);
      boost::filesystem::create_directory(m_path);
    }

    leveldb::Options options;
    options.create_if_missing = true;
    options.compression = leveldb::kNoCompression;

    m_symtab = OpenDB(options, m_path / "symtab");
    m_shltab = OpenDB(options, m_path / "shltab");

    options.comparator = &SYSTEM_CLOCK_COMPARATOR;
    if (m_cache_size_mb) {
      options.block_cache = leveldb::NewLRUCache(m_cache_size_mb * 1024 * 1024);
      m_dcache = unique_ptr<leveldb::Cache>(options.block_cache);
    }
    m_dattab = OpenDB(options, m_path / "dattab");
  }

  unique_ptr<leveldb::DB> LevelDBStorage::OpenDB(const leveldb::Options &options,
          const boost::filesystem::path &path) const {
    leveldb::DB *db = NULL;
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &db);
    if (!status.ok()) {
      BOOST_THROW_EXCEPTION(std::runtime_error(status.ToString()));
    }
    return unique_ptr<leveldb::DB>(db);
  }

  void LevelDBStorage::Add(uintptr_t address,
          std::size_t memsize,
          const CallStackInfo &callstack,
          const chrono::system_clock::time_point &timestamp) {}

  bool LevelDBStorage::Remove(uintptr_t address) {
    return false;
  }

  bool LevelDBStorage::UpdateSize(uintptr_t address, std::size_t memsize) {
    return false;
  }

  void LevelDBStorage::Clear() {}

  void LevelDBStorage::Flush() {}

  std::string LevelDBStorage::GetName() const {
    return m_path.string();
  }

  unique_ptr<MappedStorage> NewLevelDBStorage(const char *path, std::size_t cache_size_mb) {
    return unique_ptr<MappedStorage>(new LevelDBStorage(path, cache_size_mb));
  }

}  // memhook

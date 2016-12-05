#include "leveldb_storage.h"

#include <boost/filesystem/operations.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/throw_exception.hpp>

#include <leveldb/comparator.h>

namespace memhook
{
namespace
{
    class SystemClockComparator: public leveldb::Comparator
    {
    public:
        int Compare(const leveldb::Slice& lhs, const leveldb::Slice& rhs) const
        {
            const boost::chrono::system_clock::rep lhs_rep =
                    *reinterpret_cast<const boost::chrono::system_clock::rep *>(lhs.data());

            const boost::chrono::system_clock::rep rhs_rep =
                    *reinterpret_cast<const boost::chrono::system_clock::rep *>(rhs.data());

            if (lhs_rep < rhs_rep)
                return -1;

            if (lhs_rep > rhs_rep)
                return 1;

            return 0;
        }

        const char* Name() const { return "SystemClockComparator"; }
        void FindShortestSeparator(std::string *, const leveldb::Slice &) const {}
        void FindShortSuccessor(std::string *) const {}
    };

    SystemClockComparator SYSTEM_CLOCK_COMPARATOR;
} // namespace

LevelDBStorage::LevelDBStorage(const char *path, std::size_t cache_size_mb)
    : path_(path)
    , cache_size_mb_(cache_size_mb)
{
    if (!boost::filesystem::is_directory(path_))
    {
        if (boost::filesystem::exists(path_))
            boost::filesystem::remove(path_);
        boost::filesystem::create_directory(path_);
    }

    leveldb::Options options;
    options.create_if_missing = true;
    options.compression = leveldb::kNoCompression;

    symtab_ = OpenDB(options, path_ / "symtab");
    shltab_ = OpenDB(options, path_ / "shltab");

    options.comparator = &SYSTEM_CLOCK_COMPARATOR;
    if (cache_size_mb_)
    {
        options.block_cache = leveldb::NewLRUCache(cache_size_mb_ * 1024*1024);
        dcache_ = unique_ptr<leveldb::Cache>(options.block_cache);
    }
    dattab_ = OpenDB(options, path_ / "dattab");
}

unique_ptr<leveldb::DB> LevelDBStorage::OpenDB(const leveldb::Options &options,
        const boost::filesystem::path &path) const
{
    leveldb::DB *db = NULL;
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &db);
    if (!status.ok())
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(status.ToString()));
    }
    return unique_ptr<leveldb::DB>(db);
}

void LevelDBStorage::Insert(uintptr_t address, std::size_t memsize,
    const boost::chrono::system_clock::time_point &timestamp,
    const CallStackInfo &callstack)
{

}

bool LevelDBStorage::Erase(uintptr_t address)
{

}

bool LevelDBStorage::UpdateSize(uintptr_t address, std::size_t memsize)
{

}

void LevelDBStorage::Clear()
{

}

void LevelDBStorage::Flush()
{

}

std::string LevelDBStorage::GetName() const
{
    return path_.string();
}

unique_ptr<MappedStorage> NewLevelDBStorage(const char *path, std::size_t cache_size_mb)
{
    return unique_ptr<MappedStorage>(new LevelDBStorage(path, cache_size_mb));
}

} // memhook

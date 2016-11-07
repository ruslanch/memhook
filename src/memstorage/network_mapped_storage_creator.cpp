#include "common.h"
#include "network_mapped_storage.h"

#include <memhook/mapped_storage_creator.h>

namespace memhook
{

class NetworkMappedStorageCreator : public MappedStorageCreator
{
public:
    NetworkMappedStorageCreator(const char *host, int port);
    unique_ptr<MappedStorage> New(uintptr_t guide) const;

private:
    std::string host_;
    int port_;
};

NetworkMappedStorageCreator::NetworkMappedStorageCreator(const char *host, int port)
        : host_(host), port_(port)
{}

unique_ptr<MappedStorage> NetworkMappedStorageCreator::New(uintptr_t) const
{
    return unique_ptr<MappedStorage>(new NetworkMappedStorage(host_.c_str(), port_));
}

unique_ptr<MappedStorageCreator> NewNetworkMappedStorageCreator(const char *host, int port)
{
    return unique_ptr<MappedStorageCreator>(new NetworkMappedStorageCreator(host, port));
}

} // memhook

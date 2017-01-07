#ifndef MEMHOOK_SRC_MEMHOOKCORE_BASIC_MAPPED_STORAGE_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOKCORE_BASIC_MAPPED_STORAGE_H_INCLUDED

#include "common.h"

#include <memhook/scoped_signal_lock.h>
#include <memhook/mapped_storage.h>
#include <memhook/mapping_traits.h>

#include <boost/interprocess/creation_tags.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace memhook {
  template <typename Traits, typename ContainerT>
  class BasicMappedStorage : public MappedStorage {
  public:
    BasicMappedStorage(const char *container_name, const char *name, std::size_t size)
        : m_name(name)
        , m_size(size)
        , m_cleaner(m_name.c_str())
        , m_segment(boost::interprocess::create_only, m_name.c_str(), size)
        , m_allocator_instance(m_segment.get_segment_manager())
        , m_container(m_segment.template construct<ConcreteMappedContainer>(
                  container_name)(m_allocator_instance)) {}

    void Add(uintptr_t address, std::size_t memsize, const CallStackInfo &callstack,
          const chrono::system_clock::time_point &timestamp) {
      ScopedSignalLock signal_lock;
      boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*m_container);
      boost::for_each(callstack, SymbolTableUpdater(*m_container, m_allocator_instance));
      m_container->indexed_container.emplace(address, memsize, timestamp, callstack,
              m_allocator_instance);
    }

    bool Remove(uintptr_t address) {
      ScopedSignalLock signal_lock;
      boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*m_container);
      typedef typename IndexedContainer::template nth_index<0>::type Index0;
      Index0 &idx = boost::get<0>(m_container->indexed_container);
      const typename Index0::iterator iter = idx.find(address);
      if (iter == idx.end())
        return false;
      idx.erase(iter);
      return true;
    }

    bool UpdateSize(uintptr_t address, std::size_t memsize) {
      ScopedSignalLock signal_lock;
      boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*m_container);
      typedef typename IndexedContainer::template nth_index<0>::type Index0;
      Index0 &idx = boost::get<0>(m_container->indexed_container);
      const typename Index0::iterator iter = idx.find(address);
      if (iter == idx.end())
        return false;
      idx.modify(iter, boost::lambda::bind(&TraceInfoBase::memsize, boost::lambda::_1) = memsize);
      return true;
    }

    void Clear() {
      ScopedSignalLock signal_lock;
      boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*m_container);
      m_container->indexed_container.clear();
    }

    void Flush() {};

    std::string GetName() const {
      return m_name;
    }

  private:
    typedef MappingCleaner<Traits> Cleaner;
    typedef ContainerT ConcreteMappedContainer;
    typedef typename Traits::Segment Segment;
    typedef typename Traits::GenericAllocator GenericAllocator;
    typedef typename ConcreteMappedContainer::IndexedContainer IndexedContainer;
    typedef typename ConcreteMappedContainer::SymbolTable SymbolTable;
    typedef typename ConcreteMappedContainer::String String;
    typedef MappedTraceInfo<Traits> TraceInfo;

    struct SymbolTableUpdater {
      ConcreteMappedContainer &m_container;
      GenericAllocator        &m_allocator_instance;

      SymbolTableUpdater(ConcreteMappedContainer &container,
              GenericAllocator &allocator_instance)
          : m_container(container)
          , m_allocator_instance(allocator_instance) {}

      void operator()(const CallStackInfoItem &item) {
        typename SymbolTable::const_iterator iter = m_container.symtab.find(item.ip);
        if (iter == m_container.symtab.end()) {
          String symbol(item.procname.begin(), item.procname.end(), m_allocator_instance);
          m_container.symtab.emplace(item.ip, boost::move(symbol));
          iter = m_container.shltab.find(item.shl_addr);
          if (iter == m_container.shltab.end()) {
            String shl_path(item.shl_path.begin(), item.shl_path.end(),
                    m_allocator_instance);
            m_container.shltab.emplace(item.shl_addr, boost::move(shl_path));
          }
        }
      }
    };

    std::string m_name;
    std::size_t m_size;

    Cleaner m_cleaner;
    Segment m_segment;
    GenericAllocator m_allocator_instance;
    ConcreteMappedContainer *m_container;
  };
}

#endif

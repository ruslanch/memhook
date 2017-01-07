#ifndef MEMHOOK_INCLUDE_MAPPING_TRAITS_H_INCLUDED
#define MEMHOOK_INCLUDE_MAPPING_TRAITS_H_INCLUDED

#include <memhook/common.h>
#include <memhook/callstack.h>
#include <memhook/traceinfo.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/container/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/cstdint.hpp>

namespace memhook {
  template <typename SegmentT>
  struct MappingTraits {
    typedef SegmentT                                             Segment;
    typedef typename SegmentT::segment_manager                   SegmentManager;
    typedef boost::interprocess::allocator<void, SegmentManager> GenericAllocator;
  };

  template <typename Traits>
  struct MappedTraceInfo
      : BasicTraceInfo<
          boost::interprocess::allocator<
            TraceInfoCallStackItem,
            typename Traits::SegmentManager
          >
        > {
    typedef BasicTraceInfo<
      boost::interprocess::allocator<
        TraceInfoCallStackItem,
        typename Traits::SegmentManager
      >
    > BaseType;

    typedef typename Traits::GenericAllocator GenericAllocator;

    explicit MappedTraceInfo(const GenericAllocator &allocator_instance)
        : BaseType(allocator_instance) {}

      MappedTraceInfo(uintptr_t address, size_t memsize,
          const chrono::system_clock::time_point &timestamp,
          const CallStackInfo &callstack, const GenericAllocator &allocator_instance)
          : BaseType(address, memsize, timestamp, callstack, allocator_instance) {}
  };

  template <typename Traits, typename CharT = char>
  struct MappedContainerBase : boost::interprocess::interprocess_mutex {
    typedef typename Traits::SegmentManager   SegmentManager;
    typedef typename Traits::GenericAllocator GenericAllocator;
    typedef MappedTraceInfo<Traits>           TraceInfo;

    typedef boost::container::basic_string<
            CharT,
            std::char_traits<CharT>,
            boost::interprocess::allocator<CharT, SegmentManager>
          > String;

    typedef std::pair<
            const uintptr_t,
            String
          > SymbolTableValue;

    typedef boost::unordered_map<
            uintptr_t,
            String,
            boost::hash<uintptr_t>,
            std::equal_to<uintptr_t>,
            boost::interprocess::allocator<SymbolTableValue, SegmentManager>
          > SymbolTable;

    SymbolTable symtab;
    SymbolTable shltab;

    explicit MappedContainerBase(const GenericAllocator &allocator_instance)
        : symtab(allocator_instance)
        , shltab(allocator_instance) {}
  };

  template <typename Traits, typename CharT = char>
  struct MappedContainer : MappedContainerBase<Traits, CharT> {
    typedef MappedContainerBase<Traits, CharT> BaseType;

    using typename BaseType::SegmentManager;
    using typename BaseType::GenericAllocator;
    using typename BaseType::TraceInfo;

    typedef boost::multi_index_container<
      TraceInfo,
      boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
          boost::multi_index::member<TraceInfoBase,
            uintptr_t, &TraceInfoBase::address
          >
        >,
        boost::multi_index::ordered_non_unique<
          boost::multi_index::member<TraceInfoBase,
            chrono::system_clock::time_point, &TraceInfoBase::timestamp
          >
        >,
        boost::multi_index::ordered_non_unique<
          boost::multi_index::member<TraceInfoBase,
            std::size_t, &TraceInfoBase::memsize
          >
        >
      >,
      boost::interprocess::allocator<TraceInfo, SegmentManager>
    > IndexedContainer;

    IndexedContainer indexed_container;

    explicit MappedContainer(const GenericAllocator &allocator_instance)
        : BaseType(allocator_instance)
        , indexed_container(typename IndexedContainer::ctor_args_list(), allocator_instance) {}
  };

// template <typename Traits, typename CharT = char>
// struct SimpleMappedContainer : MappedContainerBase {
//   typedef boost::multi_index_container<
//     StatiTraceInfo,
//     boost::multi_index::indexed_by<
//       boost::multi_index::ordered_unique<
//         boost::multi_index::member<TraceInfoBase,
//           uintptr_t, &TraceInfoBase::address
//         >
//       >
//     >,
//     boost::interprocess::allocator<TraceInfo, SegmentManager>
//   > IndexedContainer;

//   IndexedContainer indexed_container;

//   explicit SimpleMappedContainer(const GenericAllocator &allocator_instance)
//       : MappedContainerBase(allocator_instance)
//       , indexed_container(typename IndexedContainer::ctor_args_list(), allocator_instance) {}
// };

  template <typename Traits>
  class MappingCleaner;
}

#endif

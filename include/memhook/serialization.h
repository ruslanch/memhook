#ifndef MEMHOOK_INCLUDE_SERIALIZATION_H_INCLUDED
#define MEMHOOK_INCLUDE_SERIALIZATION_H_INCLUDED

#include <memhook/common.h>
#include <memhook/byteswap.h>

#include <boost/asio/buffer.hpp>
#include <boost/container/list.hpp>
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/type_index.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/enable_if.hpp>

namespace memhook {
  namespace serialization {
    template <typename SeqT>
    struct ReserveSeqCapacity {
      typedef SeqT Seq;
      static void Call(Seq &seq, std::size_t size) {}
    };

    template <typename T, typename U, typename A>
    struct ReserveSeqCapacity<boost::container::basic_string<T, U, A> > {
      typedef boost::container::basic_string<T, U, A> Seq;
      static void Call(Seq &seq, std::size_t size) {
        seq.reserve(size);
      }
    };

    template <typename T, typename A>
    struct ReserveSeqCapacity<boost::container::vector<T, A> > {
      typedef boost::container::vector<T, A> Seq;
      static void Call(Seq &seq, std::size_t size) {
        seq.reserve(size);
      }
    };

    template <typename K, typename V, typename H, typename E, typename A>
    struct ReserveSeqCapacity<boost::unordered_map<K, V, H, E, A> > {
      typedef boost::unordered_map<K, V, H, E, A> Seq;
      static void Call(Seq &seq, std::size_t size) {
        seq.rehash(ceil(size / seq.max_load_factor()));
      }
    };

    template <typename T, typename Enable = void>
    struct ByteOrderingConv;

    template <typename T>
    struct ByteOrderingConv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint8_t)>::type> {
      typedef uint8_t Type;
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type hton(Type val) {
        return val;
      }
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type ntoh(Type val) {
        return val;
      }
    };

    template <typename T>
    struct ByteOrderingConv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint16_t)>::type> {
      typedef uint16_t Type;
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type hton(Type val) {
        return hton_16(val);
      }
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type ntoh(Type val) {
        return ntoh_16(val);
      }
    };

    template <typename T>
    struct ByteOrderingConv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint32_t)>::type> {
      typedef uint32_t Type;
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type hton(Type val) {
        return hton_32(val);
      }
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type ntoh(Type val) {
        return ntoh_32(val);
      }
    };

    template <typename T>
    struct ByteOrderingConv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint64_t)>::type> {
      typedef uint64_t Type;
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type hton(Type val) {
        return hton_64(val);
      }
      static BOOST_FORCEINLINE BOOST_CONSTEXPR Type ntoh(Type val) {
        return ntoh_64(val);
      }
    };

    template <typename T>
    BOOST_FORCEINLINE BOOST_CONSTEXPR typename ByteOrderingConv<T>::Type hton(T val) {
      return ByteOrderingConv<T>::hton(static_cast<typename ByteOrderingConv<T>::Type>(val));
    }

    template <typename T>
    BOOST_FORCEINLINE BOOST_CONSTEXPR typename ByteOrderingConv<T>::Type ntoh(T val) {
      return ByteOrderingConv<T>::ntoh(static_cast<typename ByteOrderingConv<T>::Type>(val));
    }

    template <typename ClassT>
    struct WriterImpl {
      template <typename T>
      typename boost::enable_if<boost::fusion::traits::is_sequence<T> >::type operator()(
              const T &val) const {
        boost::fusion::for_each(val, (*static_cast<const ClassT *>(this)));
      }

      template <typename T> BOOST_CONSTEXPR
      typename boost::enable_if<boost::is_integral<T> >::type operator()(T val) const {
        static_cast<const ClassT *>(this)->Call(val);
      }

      template <typename T> BOOST_CONSTEXPR
      typename boost::enable_if<boost::is_enum<T> >::type operator()(const T &val) const {
        CallObj(static_cast<int32_t>(val));
      }

      template <typename U, typename A>
      void operator()(const boost::container::basic_string<char, U, A> &val) const {
        static_cast<const ClassT *>(this)->Call(val);
      }

      template <typename T, typename U, typename A>
      void operator()(const boost::container::basic_string<T, U, A> &val) const {
        CallSeq(val);
      }

      template <typename T, typename A>
      void operator()(const boost::container::vector<T, A> &val) const {
        CallSeq(val);
      }

      template <typename T, typename A>
      void operator()(const boost::container::list<T, A> &val) const {
        CallSeq(val);
      }

      template <typename K, typename V, typename H, typename E, typename A>
      void operator()(const boost::unordered_map<K, V, H, E, A> &val) const {
        CallSeq(val);
      }

      template <typename T, typename U>
      void operator()(const std::pair<T, U> &val) const {
        CallObj(val.first);
        CallObj(val.second);
      }

      template <typename Clock, typename Duration>
      void operator()(const boost::chrono::time_point<Clock, Duration> &val) const {
        CallObj(val.time_since_epoch().count());
      }

    private:
      template <typename T>
      void CallObj(const T &val) const {
        (*static_cast<const ClassT *>(this))(val);
      }

      template <typename SeqT>
      void CallSeq(const SeqT &seq) const {
        CallObj(static_cast<uint32_t>(seq.size()));
        boost::for_each(seq, (*static_cast<const ClassT *>(this)));
      }
    };

    class SizeOf : public WriterImpl<SizeOf> {
      std::size_t &size_;

    public:
      explicit SizeOf(std::size_t &size)
          : size_(size) {}

      template <typename T>
      void Call(T val) const {
        size_ += sizeof(val);
      }

      template <typename U, typename A>
      void Call(const boost::container::basic_string<char, U, A> &val) const {
        const uint32_t size = static_cast<uint32_t>(val.size());
        Call(size);
        size_ += size;
      }

      std::size_t GetSize() const {
        return size_;
      }
    };

    class BufferWriter : public WriterImpl<BufferWriter> {
      boost::asio::mutable_buffer &buf_;

    public:
      explicit BufferWriter(boost::asio::mutable_buffer &buf)
          : buf_(buf) {}

      template <typename T>
      void Call(T val) const {
        const typename ByteOrderingConv<T>::Type val_copy = hton(val);
        boost::asio::buffer_copy(buf_, boost::asio::buffer(&val_copy, sizeof val_copy));
        buf_ = buf_ + sizeof val_copy;
      }

      template <typename U, typename A>
      void Call(const boost::container::basic_string<char, U, A> &val) const {
        const uint32_t size = static_cast<uint32_t>(val.size());
        Call(size);
        if (size != 0) {
          boost::asio::buffer_copy(buf_, boost::asio::buffer(val.data(), size));
          buf_ = buf_ + size;
        }
      }
    };

    template <typename ClassT>
    class ReaderImpl {
    public:
      template <typename T>
      typename boost::enable_if<boost::fusion::traits::is_sequence<T> >::type operator()(
              T &val) const {
        boost::fusion::for_each(val, (*static_cast<const ClassT *>(this)));
      }

      template <typename T>
      BOOST_CONSTEXPR typename boost::enable_if<boost::is_integral<T> >::type operator()(
              T &val) const {
        static_cast<const ClassT *>(this)->Call(val);
      }

      template <typename T>
      BOOST_CONSTEXPR typename boost::enable_if<boost::is_enum<T> >::type operator()(T &val) const {
        typename ByteOrderingConv<T>::Type tmp_val = 0;
        CallObj(tmp_val);
        val = static_cast<T>(tmp_val);
      }

      template <typename U, typename A>
      void operator()(boost::container::basic_string<char, U, A> &val) const {
        static_cast<const ClassT *>(this)->Call(val);
      }

      template <typename T, typename U, typename A>
      void operator()(boost::container::basic_string<T, U, A> &val) const {
        CallSeq(val);
      }

      template <typename T, typename A>
      void operator()(boost::container::vector<T, A> &val) const {
        CallSeq(val);
      }

      template <typename T, typename A>
      void operator()(boost::container::list<T, A> &val) const {
        CallSeq(val);
      }

      template <typename K, typename V, typename H, typename E, typename A>
      void operator()(boost::unordered_map<K, V, H, E, A> &val) const {
        CallSeq(val);
      }

      template <typename T, typename U>
      void operator()(std::pair<T, U> &val) const {
        CallObj(val.first);
        CallObj(val.second);
      }

      template <typename Clock, typename Duration>
      void operator()(boost::chrono::time_point<Clock, Duration> &val) const {
        typename boost::chrono::time_point<Clock, Duration>::rep rep = 0;
        CallObj(rep);
        val = boost::chrono::time_point<Clock, Duration>(Duration(rep));
      }

    private:
      template <typename T>
      void CallObj(T &val) const {
        (*static_cast<const ClassT *>(this))(val);
      }

      template <typename SeqT>
      void CallSeq(SeqT &seq) const {
        seq.clear();

        uint32_t size = 0;
        CallObj(size);

        if (size != 0) {
          ReserveSeqCapacity<SeqT>::Call(seq, size);
          for (; size; --size) {
            typename SeqT::value_type val;
            CallObj(val);
            seq.insert(seq.end(), val);
          }
        }
      }
    };

    class BufferReader : public ReaderImpl<BufferReader> {
      boost::asio::const_buffer &buf_;

    public:
      explicit BufferReader(boost::asio::const_buffer &buf)
          : buf_(buf) {}

      template <typename T>
      void Call(T &val) const {
        val = static_cast<T>(ntoh(*boost::asio::buffer_cast<
                const typename ByteOrderingConv<T>::Type *
            >(buf_)));
        buf_ = buf_ + sizeof val;
      }

      template <typename U, typename A>
      void Call(boost::container::basic_string<char, U, A> &val) const {
        uint32_t size = 0;
        Call(size);

        val.assign(boost::asio::buffer_cast<char const *>(buf_), size);
        buf_ = buf_ + size;
      }
    };

    template <typename T>
    inline std::size_t GetSize(const T &val) {
      std::size_t size = 0;
      SizeOf h(size);
      boost::fusion::for_each(val, h);
      return h.GetSize();
    }

    template <typename T>
    inline void Write(boost::asio::mutable_buffer &buf, const T &val) {
      BufferWriter h(buf);
      boost::fusion::for_each(val, h);
    }

    template <typename T>
    inline void Read(boost::asio::const_buffer &buf, T &val) {
      BufferReader h(buf);
      boost::fusion::for_each(val, h);
    }

  }  // serialization
}  // memhook

#endif

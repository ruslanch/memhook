#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/array.hpp>
#include <boost/type_index.hpp>

#define _BSD_SOURCE
#include <endian.h>

namespace memhook {

template <typename T, typename Enable = void>
struct network_ordering_conv;

template <typename T>
struct network_ordering_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint16_t)>::type> {
    typedef uint16_t type;
    static type hton(type val) { return htobe16(val); }
    static type ntoh(type val) { return be16toh(val); }
};

template <typename T>
struct network_ordering_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint32_t)>::type> {
    typedef uint32_t type;
    static type hton(type val) { return htobe32(val); }
    static type ntoh(type val) { return be32toh(val); }
};

template <typename T>
struct network_ordering_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint64_t)>::type> {
    typedef uint64_t type;
    static type hton(type val) { return htobe64(val); }
    static type ntoh(type val) { return be64toh(val); }
};

template <typename T> inline
typename network_ordering_conv<T>::type hton(T val) {
    return network_ordering_conv<T>::hton(static_cast<typename network_ordering_conv<T>::type>(val));
}

template <typename T> inline
typename network_ordering_conv<T>::type ntoh(T val) {
    return network_ordering_conv<T>::ntoh(static_cast<typename network_ordering_conv<T>::type>(val));
}

struct net_proto_writer {
    std::streambuf *sbuf;

    explicit net_proto_writer(std::streambuf *sbuf)
            : sbuf(sbuf) {}

    template <typename T>
    typename enable_if<fusion::traits::is_sequence<T> >::type
    operator()(const T &val) const {
        fusion::for_each(val, *this);
    }

    template <typename T>
    typename enable_if<is_integral<T> >::type
    operator()(const T &val) const {
        const typename network_ordering_conv<T>::type n = hton(val);
        std::size_t bytes_transferred = sbuf->sputn(reinterpret_cast<const char *>(&n), sizeof(n));
        if (bytes_transferred != sizeof(n))
            BOOST_THROW_EXCEPTION(std::length_error("bytes_transferred != sizeof(n)"));
    }

    template <typename T>
    typename enable_if<is_enum<T> >::type
    operator()(const T &val) const {
        const typename network_ordering_conv<T>::type n = static_cast<T>(val);
        (*this)(n);
    }

    template <typename T, typename U, typename A>
    void operator()(const container::basic_string<T, U, A> &val) const {
        (*this)(val.size());
        std::size_t bytes_transferred = sbuf->sputn(val.data(), val.size());
        if (bytes_transferred != val.size())
            BOOST_THROW_EXCEPTION(std::length_error("bytes_transferred != val.size()"));
    }

    template <typename T, typename A>
    void operator()(const container::vector<T, A> &val) const {
        (*this)(val.size());
        for_each(val, *this);
    }

    template <typename K, typename V, typename H, typename E, typename A>
    void operator()(const unordered_map<K, V, H, E, A> &val) const {
        (*this)(val.size());
        for_each(val, *this);
    }

    template <typename T, typename U>
    void operator()(const std::pair<T, U> &val) const {
        (*this)(val.first);
        (*this)(val.second);
    }

    template <typename Clock, typename Duration>
    void operator()(const chrono::time_point<Clock, Duration> &val) const {
        (*this)(val.time_since_epoch().count());
    }
};

struct net_proto_reader {
    std::streambuf *sbuf;

    explicit net_proto_reader(std::streambuf *sbuf)
            : sbuf(sbuf) {}

    template <typename T>
    typename enable_if<fusion::traits::is_sequence<T> >::type
    operator()(T &val) const {
        fusion::for_each(val, *this);
    }

    template <typename T>
    typename enable_if<is_integral<T> >::type
    operator()(T &val) const {
        typename network_ordering_conv<T>::type n = 0;
        const std::size_t bytes_transferred = sbuf->sgetn(reinterpret_cast<char *>(&n), sizeof(n));
        if (bytes_transferred != sizeof(n))
            BOOST_THROW_EXCEPTION(std::length_error("bytes_transferred != sizeof(n)"));
        val = static_cast<T>(ntoh(n));
    }

    template <typename T>
    typename enable_if<is_enum<T> >::type
    operator()(T &val) const {
        typename network_ordering_conv<T>::type n = 0;
        (*this)(n);
        val = static_cast<T>(ntoh(n));
    }

    template <typename T, typename U, typename A>
    void operator()(container::basic_string<T, U, A> &val) const {
        typename container::basic_string<T, U, A>::size_type size = 0;
        (*this)(size);
        val.resize(0);
        val.reserve(size);
        array<char, 256> tmpbuf;
        while (size) {
            const std::size_t bytes_read = std::min(tmpbuf.size(), size);
            const std::size_t bytes_transferred = sbuf->sgetn(tmpbuf.data(), bytes_read);
            if (bytes_transferred != bytes_read)
                BOOST_THROW_EXCEPTION(std::length_error("bytes_transferred != sizeof(n)"));
            val.append(tmpbuf.data(), bytes_transferred);
            size -= bytes_read;
        }
    }

    template <typename T, typename A>
    void operator()(container::vector<T, A> &val) const {
        typename container::vector<T, A>::size_type size = 0;
        (*this)(size);
        size = 1;
        val.resize(0);
        val.reserve(size);
        for (; size; --size) {
            T tmp;
            (*this)(tmp);
            val.emplace_back(move(tmp));
        }
    }

    template <typename K, typename V, typename H, typename E, typename A>
    void operator()(unordered_map<K, V, H, E, A> &val) const {
        typename unordered_map<K, V, H, E, A>::size_type size = 0;
        (*this)(size);
        for (; size; --size) {
            std::pair<K, V> tmp;
            (*this)(tmp);
            val.emplace(move(tmp));
        }
    }

    template <typename T, typename U>
    void operator()(std::pair<T, U> &val) const {
        (*this)(val.first);
        (*this)(val.second);
    }

    template <typename Clock, typename Duration>
    void operator()(chrono::time_point<Clock, Duration> &val) const {
        typename chrono::time_point<Clock, Duration>::rep rep = 0;
        (*this)(rep);
        val = chrono::time_point<Clock, Duration>(Duration(rep));
    }
};

template <typename T> inline
typename enable_if<is_base_of<net_proto_tag, T> >::type
write(std::streambuf *sbuf, const T &val) {
    net_proto_writer w(sbuf);
    fusion::for_each(val, w);
}

template <typename T> inline
typename enable_if<is_base_of<net_proto_tag, T> >::type
read(std::streambuf *sbuf, T &val) {
    net_proto_reader r(sbuf);
    fusion::for_each(val, r);
}

} // memhook

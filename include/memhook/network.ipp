#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/array.hpp>
#include <boost/type_index.hpp>

#include "byteswap.hpp"

namespace memhook {
namespace network_detail {
    template <typename T, typename Enable = void>
    struct byte_ordering_conv;

    template <typename T>
    struct byte_ordering_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint16_t)>::type> {
        typedef uint16_t type;
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type hton(type val) BOOST_NOEXCEPT_OR_NOTHROW
            { return hton_16(val); }
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type ntoh(type val) BOOST_NOEXCEPT_OR_NOTHROW
            { return ntoh_16(val); }
    };

    template <typename T>
    struct byte_ordering_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint32_t)>::type> {
        typedef uint32_t type;
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type hton(type val) BOOST_NOEXCEPT_OR_NOTHROW
            { return hton_32(val); }
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type ntoh(type val) BOOST_NOEXCEPT_OR_NOTHROW
            { return ntoh_32(val); }
    };

    template <typename T>
    struct byte_ordering_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint64_t)>::type> {
        typedef uint64_t type;
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type hton(type val) BOOST_NOEXCEPT_OR_NOTHROW
            { return hton_64(val); }
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type ntoh(type val) BOOST_NOEXCEPT_OR_NOTHROW
            { return ntoh_64(val); }
    };

    template <typename T> BOOST_FORCEINLINE BOOST_CONSTEXPR
    typename byte_ordering_conv<T>::type hton(T val) BOOST_NOEXCEPT_OR_NOTHROW {
        return byte_ordering_conv<T>::hton(static_cast<typename byte_ordering_conv<T>::type>(val));
    }

    template <typename T> BOOST_FORCEINLINE BOOST_CONSTEXPR
    typename byte_ordering_conv<T>::type ntoh(T val) BOOST_NOEXCEPT_OR_NOTHROW {
        return byte_ordering_conv<T>::ntoh(static_cast<typename byte_ordering_conv<T>::type>(val));
    }

    struct size_of_helper {
        std::size_t &size_;

        size_of_helper(std::size_t &size)
            : size_(size) {}

        template <typename T>
        typename enable_if<fusion::traits::is_sequence<T> >::type
        operator()(const T &val) const BOOST_NOEXCEPT_OR_NOTHROW {
            fusion::for_each(val, *this);
        }

        template <typename T> BOOST_CONSTEXPR
        typename enable_if<is_integral<T> >::type
        operator()(const T &) const BOOST_NOEXCEPT_OR_NOTHROW {
            size_ += sizeof(T);
        }

        template <typename T> BOOST_CONSTEXPR
        typename enable_if<is_enum<T> >::type
        operator()(const T &val) const BOOST_NOEXCEPT_OR_NOTHROW {
            (*this)(static_cast<typename byte_ordering_conv<T>::type>(val));
        }

        template <typename T, typename U, typename A>
        void operator()(const container::basic_string<T, U, A> &val) const
                BOOST_NOEXCEPT_OR_NOTHROW {
            const typename container::basic_string<T, U, A>::size_type val_size = val.size();
            size_ += val_size * sizeof(T);
            (*this)(val_size);
        }

        template <typename T, typename A>
        void operator()(const container::vector<T, A> &val) const BOOST_NOEXCEPT_OR_NOTHROW {
            (*this)(typename container::vector<T, A>::size_type(0));
            for_each(val, *this);
        }

        template <typename K, typename V, typename H, typename E, typename A>
        void operator()(const unordered_map<K, V, H, E, A> &val) const BOOST_NOEXCEPT_OR_NOTHROW {
            (*this)(typename unordered_map<K, V, H, E, A>::size_type(0));
            for_each(val, *this);
        }

        template <typename T, typename U>
        void operator()(const std::pair<T, U> &val) const BOOST_NOEXCEPT_OR_NOTHROW {
            (*this)(val.first);
            (*this)(val.second);
        }

        template <typename Clock, typename Duration>
        void operator()(const chrono::time_point<Clock, Duration> &val) const
                BOOST_NOEXCEPT_OR_NOTHROW {
            (*this)(typename Duration::rep(0));
        }
    };

    struct write_helper {
        asio::mutable_buffer &buf_;

        explicit write_helper(asio::mutable_buffer &buf)
                : buf_(buf) {}

        template <typename T>
        typename enable_if<fusion::traits::is_sequence<T> >::type
        operator()(const T &val) const {
            fusion::for_each(val, *this);
        }

        template <typename T>
        typename enable_if<is_integral<T> >::type
        operator()(const T &val) const {
            const typename byte_ordering_conv<T>::type tmp = hton(val);
            asio::buffer_copy(buf_, asio::buffer(&tmp, sizeof(T)));
            buf_ = buf_ + sizeof tmp;
        }

        template <typename T>
        typename enable_if<is_enum<T> >::type
        operator()(const T &val) const {
            (*this)(static_cast<typename byte_ordering_conv<T>::type>(val));
        }

        template <typename T, typename U, typename A>
        void operator()(const container::basic_string<T, U, A> &val) const {
            const typename container::basic_string<T, U, A>::size_type size = val.size();
            (*this)(size);
            asio::buffer_copy(buf_, asio::buffer(val.c_str(), size));
            buf_ = buf_ + size;
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

    struct read_helper {
        asio::const_buffer &buf_;

        explicit read_helper(asio::const_buffer &buf)
                : buf_(buf) {}

        template <typename T>
        typename enable_if<fusion::traits::is_sequence<T> >::type
        operator()(T &val) const {
            fusion::for_each(val, *this);
        }

        template <typename T>
        typename enable_if<is_integral<T> >::type
        operator()(T &val) const {
            val = static_cast<T>(ntoh(*asio::buffer_cast<
                    const typename byte_ordering_conv<T>::type *
                >(buf_)));
            buf_ = buf_ + sizeof val;
        }

        template <typename T>
        typename enable_if<is_enum<T> >::type
        operator()(T &val) const {
            typename byte_ordering_conv<T>::type tmp = 0;
            (*this)(tmp);
            val = static_cast<T>(tmp);
        }

        template <typename T, typename U, typename A>
        void operator()(container::basic_string<T, U, A> &val) const {
            typename container::basic_string<T, U, A>::size_type size = 0;
            (*this)(size);
            val.assign(asio::buffer_cast<char const*>(buf_), size);
            buf_ = buf_ + size;
        }

        template <typename T, typename A>
        void operator()(container::vector<T, A> &val) const {
            typename container::vector<T, A>::size_type size = 0;
            (*this)(size);
            val.clear();
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

} // network_detail

template <typename T> inline
typename enable_if<is_base_of<net_proto_tag, T>, std::size_t>::type
size_of(const T &val) BOOST_NOEXCEPT_OR_NOTHROW {
    std::size_t size = 0;
    network_detail::size_of_helper h(size);
    fusion::for_each(val, h);
    return h.size_;
}

template <typename T> inline
typename enable_if<is_base_of<net_proto_tag, T> >::type
write(asio::mutable_buffer &buf, const T &val) {
    network_detail::write_helper h(buf);
    fusion::for_each(val, h);
}

template <typename T> inline
typename enable_if<is_base_of<net_proto_tag, T> >::type
read(asio::const_buffer &buf, T &val) {
    network_detail::read_helper h(buf);
    fusion::for_each(val, h);
}

} // memhook

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
    struct byte_ordering_conv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint16_t)>::type> {
        typedef uint16_t type;
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type hton(type val)
            { return hton_16(val); }
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type ntoh(type val)
            { return ntoh_16(val); }
    };

    template <typename T>
    struct byte_ordering_conv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint32_t)>::type> {
        typedef uint32_t type;
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type hton(type val)
            { return hton_32(val); }
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type ntoh(type val)
            { return ntoh_32(val); }
    };

    template <typename T>
    struct byte_ordering_conv<T, typename boost::enable_if_c<sizeof(T) == sizeof(uint64_t)>::type> {
        typedef uint64_t type;
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type hton(type val)
            { return hton_64(val); }
        static BOOST_FORCEINLINE BOOST_CONSTEXPR type ntoh(type val)
            { return ntoh_64(val); }
    };

    template <typename T> BOOST_FORCEINLINE BOOST_CONSTEXPR
    typename byte_ordering_conv<T>::type hton(T val) {
        return byte_ordering_conv<T>::hton(static_cast<typename byte_ordering_conv<T>::type>(val));
    }

    template <typename T> BOOST_FORCEINLINE BOOST_CONSTEXPR
    typename byte_ordering_conv<T>::type ntoh(T val) {
        return byte_ordering_conv<T>::ntoh(static_cast<typename byte_ordering_conv<T>::type>(val));
    }

    struct size_of_helper {
        std::size_t &size_;

        size_of_helper(std::size_t &size)
            : size_(size) {}

        template <typename T>
        typename boost::enable_if<boost::fusion::traits::is_sequence<T> >::type
        operator()(const T &val) const {
            boost::fusion::for_each(val, *this);
        }

        template <typename T> BOOST_CONSTEXPR
        typename boost::enable_if<boost::is_integral<T> >::type
        operator()(const T &) const {
            size_ += sizeof(T);
        }

        template <typename T> BOOST_CONSTEXPR
        typename boost::enable_if<boost::is_enum<T> >::type
        operator()(const T &val) const {
            (*this)(static_cast<typename byte_ordering_conv<T>::type>(val));
        }

        template <typename T, typename U, typename A>
        void operator()(const boost::container::basic_string<T, U, A> &val) const
                {
            const typename boost::container::basic_string<T, U, A>::size_type val_size = val.size();
            size_ += val_size * sizeof(T);
            (*this)(val_size);
        }

        template <typename T, typename A>
        void operator()(const boost::container::vector<T, A> &val) const {
            (*this)(typename boost::container::vector<T, A>::size_type(0));
            boost::for_each(val, *this);
        }

        template <typename K, typename V, typename H, typename E, typename A>
        void operator()(const boost::unordered_map<K, V, H, E, A> &val) const {
            (*this)(typename boost::unordered_map<K, V, H, E, A>::size_type(0));
            boost::for_each(val, *this);
        }

        template <typename T, typename U>
        void operator()(const std::pair<T, U> &val) const {
            (*this)(val.first);
            (*this)(val.second);
        }

        template <typename Clock, typename Duration>
        void operator()(const boost::chrono::time_point<Clock, Duration> &val) const
                {
            (*this)(typename Duration::rep(0));
        }
    };

    struct write_helper {
        boost::asio::mutable_buffer &buf_;

        explicit write_helper(boost::asio::mutable_buffer &buf)
                : buf_(buf) {}

        template <typename T>
        typename boost::enable_if<boost::fusion::traits::is_sequence<T> >::type
        operator()(const T &val) const {
            boost::fusion::for_each(val, *this);
        }

        template <typename T>
        typename boost::enable_if<boost::is_integral<T> >::type
        operator()(const T &val) const {
            const typename byte_ordering_conv<T>::type tmp = hton(val);
            boost::asio::buffer_copy(buf_, boost::asio::buffer(&tmp, sizeof(T)));
            buf_ = buf_ + sizeof tmp;
        }

        template <typename T>
        typename boost::enable_if<boost::is_enum<T> >::type
        operator()(const T &val) const {
            (*this)(static_cast<typename byte_ordering_conv<T>::type>(val));
        }

        template <typename T, typename U, typename A>
        void operator()(const boost::container::basic_string<T, U, A> &val) const {
            const typename boost::container::basic_string<T, U, A>::size_type size = val.size();
            (*this)(size);
            boost::asio::buffer_copy(buf_, boost::asio::buffer(val.c_str(), size));
            buf_ = buf_ + size;
        }

        template <typename T, typename A>
        void operator()(const boost::container::vector<T, A> &val) const {
            (*this)(val.size());
            boost::for_each(val, *this);
        }

        template <typename K, typename V, typename H, typename E, typename A>
        void operator()(const boost::unordered_map<K, V, H, E, A> &val) const {
            (*this)(val.size());
            boost::for_each(val, *this);
        }

        template <typename T, typename U>
        void operator()(const std::pair<T, U> &val) const {
            (*this)(val.first);
            (*this)(val.second);
        }

        template <typename Clock, typename Duration>
        void operator()(const boost::chrono::time_point<Clock, Duration> &val) const {
            (*this)(val.time_since_epoch().count());
        }
    };

    struct read_helper {
        boost::asio::const_buffer &buf_;

        explicit read_helper(boost::asio::const_buffer &buf)
                : buf_(buf) {}

        template <typename T>
        typename boost::enable_if<boost::fusion::traits::is_sequence<T> >::type
        operator()(T &val) const {
            boost::fusion::for_each(val, *this);
        }

        template <typename T>
        typename boost::enable_if<boost::is_integral<T> >::type
        operator()(T &val) const {
            val = static_cast<T>(ntoh(*boost::asio::buffer_cast<
                    const typename byte_ordering_conv<T>::type *
                >(buf_)));
            buf_ = buf_ + sizeof val;
        }

        template <typename T>
        typename boost::enable_if<boost::is_enum<T> >::type
        operator()(T &val) const {
            typename byte_ordering_conv<T>::type tmp = 0;
            (*this)(tmp);
            val = static_cast<T>(tmp);
        }

        template <typename T, typename U, typename A>
        void operator()(boost::container::basic_string<T, U, A> &val) const {
            typename boost::container::basic_string<T, U, A>::size_type size = 0;
            (*this)(size);
            val.assign(boost::asio::buffer_cast<char const*>(buf_), size);
            buf_ = buf_ + size;
        }

        template <typename T, typename A>
        void operator()(boost::container::vector<T, A> &val) const {
            typename boost::container::vector<T, A>::size_type size = 0;
            (*this)(size);
            val.clear();
            val.reserve(size);
            for (; size; --size) {
                T tmp;
                (*this)(tmp);
                val.emplace_back(boost::move(tmp));
            }
        }

        template <typename K, typename V, typename H, typename E, typename A>
        void operator()(boost::unordered_map<K, V, H, E, A> &val) const {
            typename boost::unordered_map<K, V, H, E, A>::size_type size = 0;
            (*this)(size);
            for (; size; --size) {
                std::pair<K, V> tmp;
                (*this)(tmp);
                val.emplace(boost::move(tmp));
            }
        }

        template <typename T, typename U>
        void operator()(std::pair<T, U> &val) const {
            (*this)(val.first);
            (*this)(val.second);
        }

        template <typename Clock, typename Duration>
        void operator()(boost::chrono::time_point<Clock, Duration> &val) const {
            typename boost::chrono::time_point<Clock, Duration>::rep rep = 0;
            (*this)(rep);
            val = boost::chrono::time_point<Clock, Duration>(Duration(rep));
        }
    };

} // network_detail

template <typename T> inline
typename boost::enable_if<boost::is_base_of<net_proto_tag, T>, std::size_t>::type
size_of(const T &val) {
    std::size_t size = 0;
    network_detail::size_of_helper h(size);
    boost::fusion::for_each(val, h);
    return h.size_;
}

template <typename T> inline
typename boost::enable_if<boost::is_base_of<net_proto_tag, T> >::type
write(boost::asio::mutable_buffer &buf, const T &val) {
    network_detail::write_helper h(buf);
    boost::fusion::for_each(val, h);
}

template <typename T> inline
typename boost::enable_if<boost::is_base_of<net_proto_tag, T> >::type
read(boost::asio::const_buffer &buf, T &val) {
    network_detail::read_helper h(buf);
    boost::fusion::for_each(val, h);
}

} // memhook

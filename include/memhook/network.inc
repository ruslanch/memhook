#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>

#define _BSD_SOURCE
#include <endian.h>

namespace memhook {

template <typename T, typename Enable = void>
struct network_conv;

template <typename T>
struct network_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint16_t)>::type> {
    typedef uint16_t type;
    static type hton(type val) { return htobe16(val); }
    static type ntoh(type val) { return be16toh(val); }
};

template <typename T>
struct network_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint32_t)>::type> {
    typedef uint32_t type;
    static type hton(type val) { return htobe32(val); }
    static type ntoh(type val) { return be32toh(val); }
};

template <typename T>
struct network_conv<T, typename enable_if_c<sizeof(T) == sizeof(uint64_t)>::type> {
    typedef uint64_t type;
    static type hton(type val) { return htobe64(val); }
    static type ntoh(type val) { return be64toh(val); }
};

template <typename T> inline
typename network_conv<T>::type hton(T val) {
    return network_conv<T>::hton(static_cast<typename network_conv<T>::type>(val));
}

template <typename T> inline
typename network_conv<T>::type ntoh(T val) {
    return network_conv<T>::ntoh(static_cast<typename network_conv<T>::type>(val));
}

struct network_storage_req_writer {
    std::ostream &os_;

    explicit network_storage_req_writer(std::ostream &os)
            : os_(os) {}

    template <typename T>
    typename enable_if<is_integral<T> >::type
    operator()(const T &val) const {
        const typename network_conv<T>::type n = hton(val);
        os_.write(reinterpret_cast<const char *>(&n), sizeof(n));
    }

    template <typename T>
    typename enable_if<is_enum<T> >::type
    operator()(const T &val) const {
        const typename network_conv<T>::type n = hton(val);
        os_.write(reinterpret_cast<const char *>(&n), sizeof(n));
    }

    template <typename T>
    typename enable_if<fusion::traits::is_sequence<T> >::type
    operator()(const T &val) const {
        fusion::for_each(val, *this);
    }

    template <typename T, typename A>
    void operator()(const container::vector<T, A> &vec) const {

    }

    void operator()(const system_clock::time_point &time_point) const {

    }

    template <typename T>
    typename disable_if_c<
                fusion::traits::is_sequence<T>::value || is_integral<T>::value || is_enum<T>::value
            >::type operator()(const T &val) const {
    }
};

std::ostream &operator<<(std::ostream &os, const network_storage_req &req) {
    network_storage_req_writer writer(os);
    fusion::for_each(req, writer);
    return os;
}

std::istream &operator>>(std::istream &is, network_storage_req &req) {
    return is;
}

} // memhook

#ifndef MEMHOOK_SRC_MEMHOOK_SINGLETON_H_INCLUDED
#define MEMHOOK_SRC_MEMHOOK_SINGLETON_H_INCLUDED

#include "common.h"

#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/detail/atomic_count.hpp>

namespace memhook
{

template <typename T>
class Singleton
{
public:
    static boost::intrusive_ptr<T> GetInstance()
    {
        boost::intrusive_ptr<T> p;
        if (instance_)
        {
            p.reset(instance_);
        }
        return p;
    }

    static void Initialize()
    {
        T *p = new T();
        intrusive_ptr_add_ref(p);
        instance_ = p;

        p->OnInitialize();
    }

    static void Destroy()
    {
        T *p = instance_;
        instance_ = NULL;

        if (p)
        {
            intrusive_ptr_release(p);
        }
    }

private:
    static T *instance_;
};

template <typename T>
T *Singleton<T>::instance_ = NULL;

template <typename T>
class SingletonRefCounter;

template <typename T>
void intrusive_ptr_add_ref(const SingletonRefCounter<T> *p);

template <typename T>
void intrusive_ptr_release(const SingletonRefCounter<T> *p);

template <typename T>
class SingletonRefCounter
{
    mutable boost::detail::atomic_count ref_counter_;
public:
    SingletonRefCounter() : ref_counter_(0) {}
    SingletonRefCounter(const SingletonRefCounter &) : ref_counter_(0) {}
    SingletonRefCounter &operator=(const SingletonRefCounter &rc) { return *this; }

protected:
    BOOST_DEFAULTED_FUNCTION(~SingletonRefCounter(), {})

    friend void intrusive_ptr_add_ref<T>(const SingletonRefCounter<T> *p);
    friend void intrusive_ptr_release<T>(const SingletonRefCounter<T> *p);
};

template <typename T>
inline void intrusive_ptr_add_ref(const SingletonRefCounter<T> *p)
{
    ++p->ref_counter_;
}

template <typename T>
inline void intrusive_ptr_release(const SingletonRefCounter<T> *p)
{
    if (!--p->ref_counter_)
    {
        T *ptr = const_cast<T *>(static_cast<const T *>(p));
        ptr->OnDestroy();
        delete ptr;
    }
}

template <typename T>
class SingletonImpl
    : public Singleton<T>
    , public SingletonRefCounter<T>
{};

} // memhook

#endif

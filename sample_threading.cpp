#include <boost/thread.hpp>
#include <sstream>
#include <string>
#include <netdb.h>

using namespace boost;

void job1() {
    std::string s;
    std::stringstream ss;
    for (size_t i = 0; i < 100; ++i) {
        ss.str(s);
        ss << i;
        s = ss.str();

        // if (i == (2 << 10)) {
        //     s.clear();
        //     i = 0;
        // }
    }
}

void job2() {
    for (size_t i = 0; i < 100; ++i) {
        gethostbyname("www.ru");
        gethostbyname("yandex.ru");
        gethostbyname("google.com");
    }
}

void job3() {
    std::string s = "ru30rceriotbn4w8975fh378956cituytf";
    for (size_t i = 0; i < 100; ++i) {
        gethostbyname("www.ru");
        gethostbyname("yandex.ru");
        gethostbyname("google.com");

        std::rotate(s.begin(), s.begin() + 5, s.end());
        gethostbyname(s.c_str());
    }
}

int main(int argc, char const *argv[]) {
    thread_group tg;
    for (size_t i = 0; i < 64; ++i) {
        tg.create_thread(job1);
        tg.create_thread(job2);
    }
    tg.join_all();
    return 0;
}

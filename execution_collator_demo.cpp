#include "execution_collator.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>

std::string get_tsask_title(int a, int b, int c) {
    std::ostringstream stream;

    stream
        << "["
        << std::setw(2)
        << a
        << ":"
        << std::setw(2)
        << b
        << ":"
        << std::setw(2)
        << c
        << "]";

    return stream.str();
}

void log(std::string const& task_title, std::string const& what) {
    static std::mutex mx_cout;

    auto const now = std::chrono::system_clock::now().time_since_epoch().count();

    std::lock_guard<std::mutex> cout_lock(mx_cout);
    std::cout
        << std::setw(20)
        << now
        << "    "
        << std::setw(15)
        << task_title
        << "    "
        << what
        << std::endl;
}

std::uint64_t get_request_hash(int a, int b, int c) {
    return b;
}

int execute(int a, int b, int c) {
    auto const title = get_tsask_title(a, b, c);

    log(title, "start");
    std::this_thread::sleep_for(std::chrono::seconds(c));
    log(title, "finish");

    if (b > 50) {
        throw std::invalid_argument("b must be less than 50");
    }

    return b;
}

execution_collator<int, std::uint64_t, int, int, int> collator(
    &get_request_hash,
    &execute);

void test_request(int a, int b, int c) {
    auto const title = get_tsask_title(a, b, c);

    log(title, "request");
    try {
        auto const result = collator.execute(a, b, c);
        log(title, std::string("result: ") + std::to_string(result));
    }
    catch (std::exception const& e)
    {
        log(title, std::string("exception: ") + e.what());
    }
}

struct request_t {
    int a;
    int b;
    int c;
};

int main()
{
    request_t parameters[] = {
        { 11, 10,  1 },
        { 12, 10,  2 },
        { 13, 10,  3 },
        { 21, 20,  4 },
        { 22, 20,  5 },
        { 23, 20,  6 },
        { 31, 60,  7 },
        { 32, 60,  8 },
        { 33, 60,  9 }
    };

    std::list<std::thread> threads;

    std::transform(
        std::begin(parameters),
        std::end(parameters),
        std::back_inserter(threads),
        [](request_t const& r) { return std::thread(test_request, r.a, r.b, r.c); });

    for (auto& thread : threads) {
        thread.join();
    }

    test_request(14, 10, 1);

    return 0;
}

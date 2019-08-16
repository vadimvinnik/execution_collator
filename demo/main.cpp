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

  auto const now_time = std::time(nullptr);
  auto const now_string = std::put_time(std::gmtime(&now_time), "%H:%M:%S");

  std::lock_guard<std::mutex> cout_lock(mx_cout);
  std::cout
    << std::setw(10)
    << now_string
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

async::execution_collator<int, std::uint64_t, int, int, int> collator {
  &get_request_hash,
  &execute
};

struct demo_params_t {
  int a;
  int b;
  int c;
};

void perform_demo_call(demo_params_t const& params) {
  auto const title = get_tsask_title(
    params.a,
    params.b,
    params.c);

  log(title, "request");
  try {
    auto const result = collator.execute(
      params.a,
      params.b,
      params.c);

    log(title, std::string("result: ") + std::to_string(result));
  }
  catch (std::exception const& e) {
    log(title, std::string("exception: ") + e.what());
  }
}

std::thread start_demo_thread(demo_params_t const& params) {
  return std::thread { perform_demo_call, params };
}

int main() {
  demo_params_t parameters[] = {
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
    start_demo_thread);

  for (auto& thread : threads) {
    thread.join();
  }

  perform_demo_call({ 14, 10, 1 });

  return 0;
}


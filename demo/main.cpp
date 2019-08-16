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

std::string get_task_title(int a, char b) {
  return std::to_string(a) + b;
}

void log(std::string const& task_title, std::string const& what) {
  static std::mutex mx_cout;

  auto const now_time = std::time(nullptr);
  auto const now_string = std::put_time(std::gmtime(&now_time), "%H:%M:%S");

  std::lock_guard<std::mutex> cout_lock(mx_cout);
  std::cout
    << now_string
    << "    "
    << task_title
    << "    "
    << what
    << std::endl;
}

int get_request_hash(int a, char b) {
  return a;
}

std::string execute(int a, char b) {
  auto const title = get_task_title(a, b);

  log(title, "start");
  std::this_thread::sleep_for(std::chrono::seconds(1 + (a + b) % 5));
  log(title, "finish");

  static constexpr char const* const results[] = {
    "Chatski",
    "Onegin",
    "Pechorin",
    "Oblomov"
  };

  if (a >= std::size(results)) {
    throw std::invalid_argument("a is out of bound");
  }

  return results[a];
}

using collator_t = function_wrapper::collator<std::string, int, int, char>;

collator_t collator { &get_request_hash, &execute };

struct demo_params_t {
  int a;
  char b;
};

void perform_demo_call(demo_params_t const& params) {
  auto const title = get_task_title(params.a, params.b);

  log(title, "request");
  try {
    auto const result = collator.execute(params.a, params.b);
    log(title, std::string("result: ") + result);
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
    { 1, 'a' },
    { 1, 'b' },
    { 1, 'c' },
    { 2, 'a' },
    { 2, 'b' },
    { 2, 'c' },
    { 6, 'a' },
    { 6, 'b' },
    { 6, 'c' }
  };

  std::list<std::thread> threads;

  std::cout
    << "Starting multiple simultaneous calls"
    << std::endl;

  std::transform(
    std::begin(parameters),
    std::end(parameters),
    std::back_inserter(threads),
    start_demo_thread);

  for (auto& thread : threads) {
    thread.join();
  }
  
  std::cout
    << "All calls have finished. "
    << "Performing one more"
    <<std::endl;

  perform_demo_call({ 1, 'd' });

  return 0;
}


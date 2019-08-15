#pragma once

#include <cassert>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <map>

template <typename Result, typename Key, typename... Args>
class execution_collator {
public:
  using result_t = Result;
  using key_t = Key;
  using key_extractor_t = std::function<key_t(Args...)>;
  using request_executor_t = std::function<Result(Args...)>;

  template <typename KeyExtractor, typename RequestExecutor>
  execution_collator(KeyExtractor key_extractor, RequestExecutor request_executor) :
    key_extractor_(key_extractor),
    request_executor_(request_executor)
  {}

  Result execute(Args... args) {
    key_t key = key_extractor_(args...);

    auto const execution_it = find_or_create_execution(key);
    auto& execution = execution_it->second;

    {
      std::unique_lock<std::mutex> execution_lock(execution.mutex);

      execution.cv_result_ready.wait(
        execution_lock,
        [&execution]() { return execution.can_proceed(); });

      if (!execution.has_started) {
        assert(!execution.result.has_value());
        assert(execution.exception_ptr == nullptr);

        execution.has_started = true;
        try {
          execution.result = request_executor_(args...);
        }
        catch (...) {
          execution.exception_ptr = std::current_exception();
        }

        execution.cv_result_ready.notify_all();
      }
    }

    auto const result = execution.result;
    auto const exception_ptr = execution.exception_ptr;

    assert(result.has_value() || exception_ptr != nullptr);

    {
      std::unique_lock<std::mutex> map_lock(mutex_);

      --execution.ref_count;

      if (execution.ref_count == 0) {
        executions_cache_.erase(execution_it);
      }
    }

    if (exception_ptr != nullptr) {
      assert(!execution.result.has_value());
      std::rethrow_exception(exception_ptr);
    }

    return *result;
  }

private:
  struct execution {
    execution() :
      has_started(false),
      ref_count(0),
      result(std::nullopt),
      exception_ptr(nullptr)
    {}

    bool can_proceed() const noexcept {
      return !has_started || result.has_value() || exception_ptr != nullptr;
    }

    std::mutex mutex;
    std::condition_variable cv_result_ready;
    bool has_started;
    int ref_count;
    std::optional<Result> result;
    std::exception_ptr exception_ptr;
  };

  using executions_cache_t = std::map<key_t, execution>;
  using executions_iterator_t = typename executions_cache_t::iterator;

  executions_iterator_t find_or_create_execution(key_t const& key) {
    std::unique_lock<std::mutex> map_lock(mutex_);

    auto const it = executions_cache_.emplace(
      std::piecewise_construct,
      std::make_tuple(key),
      std::make_tuple())
    .first;

    ++it->second.ref_count;

    return it;
  }

  key_extractor_t key_extractor_;
  request_executor_t request_executor_;
  std::mutex mutex_;
  executions_cache_t executions_cache_;
};

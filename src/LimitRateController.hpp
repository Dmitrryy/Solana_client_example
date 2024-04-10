#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

/// @brief Controls limit rate.
///
/// Each call wait_limit_rate consumes one limit point. If the limit is
/// exceeded, execution is blocked until the next time window.
///
/// NOTE: The implemented logic is an approximation for honest saving to an
/// array of call time points in the past.
class LimitRateController {
public:
  LimitRateController(size_t time_window_size_ms, size_t max_requests)
      : m_time_window_size(time_window_size_ms), m_max_requests(max_requests),
        m_current_request_count(0),
        m_current_window_start(std::chrono::steady_clock::now()) {}

  void wait_limit_rate() {
    std::unique_lock<std::mutex> lock(m_mutex);
    updateWindow();

    while (m_current_request_count >= m_max_requests) {
      // sleep for next time window
      std::this_thread::sleep_for(
          std::chrono::milliseconds(m_time_window_size));
      updateWindow();
    }

    ++m_current_request_count;
  }

private:
  void updateWindow() {
    auto now = std::chrono::steady_clock::now();
    if (!isInCurrentWindow()) {
      m_current_window_start = now;
      m_current_request_count = 0;
    }
  }

  bool isInCurrentWindow() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - m_current_window_start)
               .count() < m_time_window_size;
  }

  size_t m_time_window_size = 0;
  size_t m_max_requests = 0;
  std::atomic<size_t> m_current_request_count;
  std::chrono::steady_clock::time_point m_current_window_start;
  std::mutex m_mutex;
};
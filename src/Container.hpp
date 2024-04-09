#pragma once

#include <algorithm>
#include <cmath>
#include <list>
#include <mutex>
#include <iostream>

template <typename KeyTy, typename ValTy> class ConcurrentContainer final {
  std::list<std::pair<KeyTy, ValTy>> m_data;
  mutable std::mutex m_access_mutex;

  // task 3
  // FIXME: it is better to separate responsibilities
  //=----------------------------------------------------------------
  size_t m_window_elements = 0;
  // sum(x_i)
  size_t m_window_sum = 0;
  // sum(x_i^2)
  size_t m_window_square_sum = 0;
  // T
  size_t m_window_width = 0;
  std::list<std::pair<KeyTy, ValTy>>::const_iterator m_window_left_it;

public:
  ConcurrentContainer(size_t window_width = 10)
      : m_window_width(window_width), m_window_left_it(m_data.end()) {}

  template <typename KeyTy2, typename ValTy2>
  void emplace_back(KeyTy2 &&key, ValTy2 &&val) {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    // find position by Key and insert
    auto &&posIt = m_data.end();
    bool insert_inside_window = true;
    while (posIt != m_data.begin()) {
      --posIt;
      if (posIt->first <= key) {
        break;
      }
      if (posIt == m_window_left_it) {
        insert_inside_window = false;
      }
    }
    m_data.emplace(++posIt, std::forward<KeyTy2>(key),
                   std::forward<ValTy2>(val));

    std::cout << m_window_elements << std::endl;

    if (insert_inside_window) {
      ++m_window_elements;
      m_window_sum += val;
      m_window_square_sum += val * val;
      shift_window();
    }
  }

  auto size() const {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    return m_data.size();
  }

  std::pair<KeyTy, ValTy> top() {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    return m_data.back();
  }

  // task 3
  //=----------------------------------------------------------------
public:
  double standard_deviation() const {
    // D = sqrt(sum((x_i - mean)^2) / N) =
    //   = sqrt(sum(x_i^2 - 2*x_i*mean + mean^2) / N) =
    //   = sqrt(sum(x_i^2)/N - 2*mean*sum(x_i)/N + mean^2) =
    //   = sqrt(sum(x_i^2)/N - mean*mean)
    std::lock_guard<std::mutex> lock(m_access_mutex);
    auto mean = static_cast<double>(m_window_sum) / m_window_elements;
    return std::sqrt(static_cast<double>(m_window_square_sum) /
                         m_window_elements -
                     mean * mean);
  }

private:
  void shift_window() {
    // TODO: determine was inserted element inside window or not?
    if (m_data.empty()) {
      return;
    }
    // step to the last element
    if (m_window_left_it == m_data.end()) {
      --m_window_left_it;
    }

    const auto X = m_data.back().first;
    const auto X_T = X - m_window_width;

    while (m_window_left_it != m_data.end() && m_window_left_it->first < X_T) {
      auto cur_latency = m_window_left_it->second;
      --m_window_elements;
      m_window_sum -= cur_latency;
      m_window_square_sum -= cur_latency * cur_latency;

      ++m_window_left_it;
    }
  }
};

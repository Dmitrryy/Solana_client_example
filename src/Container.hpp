#pragma once

#include <cmath>
#include <list>
#include <mutex>

/// A container for storing the results in parallel, maintaining a key-sorted
/// order. The container is designed with the expectation of temporary locality
/// of incoming keys (in single-threaded execution, the key does not decrease,
/// in multithreaded execution, the key position is of the order O(P), where P
/// is the number of processes).
///
/// I stopped at this option because of the opportunity for improvement:
/// Area for improvement: When inserting, it is not necessary to block the
/// entire container, you need to block only the node where the insertion takes
/// place. Reading from a sheet can be made non-blocking (on atomics).
/// 
/// Other interesting implementations:
///    1. skip list - O(logN) insertion but without blocking entire container.
///    2. Priority queue - O(logN) insertion
template <typename KeyTy, typename ValTy> class ConcurrentContainer final {
  using DataTy = std::tuple<KeyTy, size_t, ValTy>;
  std::list<DataTy> m_data;
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
  // <key, latency, value>
  std::list<DataTy>::const_iterator m_window_left_it;

public:
  ConcurrentContainer(size_t window_width = 2)
      : m_window_width(window_width), m_window_left_it(m_data.end()) {}

  template <typename KeyTy2, typename ValTy2>
  void emplace_back(KeyTy2 &&key, ValTy2 &&val, size_t latency) {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    // find position by Key and insert
    auto &&posIt = m_data.end();
    bool insert_inside_window = true;
    while (posIt != m_data.begin()) {
      --posIt;
      if (std::get<0>(*posIt) <= key) {
        break;
      }
      if (posIt == m_window_left_it) {
        insert_inside_window = false;
      }
    }
    m_data.emplace(++posIt, std::forward<KeyTy2>(key), latency,
                   std::forward<ValTy2>(val));

    if (insert_inside_window) {
      add_to_window(latency);
      shift_window();
    }
  }

  auto size() const {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    return m_data.size();
  }

  DataTy top_newer() {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    return m_data.back();
  }

  DataTy top_older() {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    return m_data.front();
  }

  bool pop_older(DataTy &Res) {
    std::lock_guard<std::mutex> lock(m_access_mutex);
    if (m_data.empty()) {
      return false;
    }
    Res = m_data.front();

    // we should delete element from window if necessary
    if (m_window_left_it == m_data.begin()) {
      auto cur_latency = m_window_left_it->second;
      delete_from_window(cur_latency);
      ++m_window_left_it;
    }
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
  void add_to_window(size_t latency) {
    ++m_window_elements;
    m_window_sum += latency;
    m_window_square_sum += latency * latency;
  }

  void delete_from_window(size_t latency) {
    --m_window_elements;
    m_window_sum -= latency;
    m_window_square_sum -= latency * latency;
  }

  void shift_window() {
    if (m_data.empty()) {
      return;
    }
    if (m_window_left_it == m_data.end()) {
      // step to the last element
      --m_window_left_it;
    }

    const auto X = std::get<0>(m_data.back());
    const auto X_T = X - m_window_width;

    while (m_window_left_it != m_data.end() &&
           std::get<0>(*m_window_left_it) < X_T) {
      auto cur_latency = std::get<1>(*m_window_left_it);
      delete_from_window(cur_latency);

      ++m_window_left_it;
    }
  }
};

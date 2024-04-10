#pragma once

#include "cpr/response.h"
#include "cpr/status_codes.h"
#include <cpr/cpr.h>

#include <chrono>
#include <thread>
#include <utility>

class HTTPErrorHandler final {
  size_t m_attempt_count = 0;
  size_t m_max_attempts_count = 0;

public:
  HTTPErrorHandler(size_t max_attempts)
      : m_attempt_count(0), m_max_attempts_count(max_attempts) {}

  template <typename FTy> cpr::Response invoke(FTy &&F) {
    auto &&r = F();

    if (m_attempt_count >= m_max_attempts_count) {
      return r;
    }

    if (cpr::status::is_success(r.status_code)) {
      return r;
    }

    if (r.status_code == 0) {
      // probably timeout
      ++m_attempt_count;
      std::this_thread::sleep_for(std::chrono::seconds(5));
      return invoke(std::forward<FTy>(F));
    }

    if (r.status_code == cpr::status::HTTP_TOO_MANY_REQUESTS) {
      ++m_attempt_count;

      size_t sleep_sec = std::stoi(r.header["retry-after"]);
      std::this_thread::sleep_for(std::chrono::seconds(sleep_sec));
      return invoke(std::forward<FTy>(F));
    }
    // there are many more interesting errors that can be handled here
    return r;
  }
};
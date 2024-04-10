#pragma once

#include "Container.hpp"
#include "ErrorHandler.hpp"
#include "IEventHandler.hpp"
#include "LimitRateController.hpp"
#include "SolanaAPI.hpp"

#include "rapidjson/document.h"

#include <chrono>
#include <cstddef>
#include <stdexcept>

/// @brief Event handler with actions according task 2.
class DefaultEventHandler final : public IEventHandler {
  SolanaRPCClient m_client;
  std::string m_pubkey;
  // FIXME: it is bad practice to save reference in a class.
  ConcurrentContainer<size_t, size_t> &m_result_container;
  LimitRateController &m_lr_controller;

public:
  DefaultEventHandler(std::string endpoint, std::string pubkey,
                      ConcurrentContainer<size_t, size_t> &res_container,
                      LimitRateController &lr_controller)
      : m_client(std::move(endpoint)), m_pubkey(std::move(pubkey)),
        m_result_container(res_container), m_lr_controller(lr_controller) {}

  /// @brief Process \event according task2.
  /// Actions:
  /// INVOKE: Execute the GET method implemented in Point 1 in the background
  /// upon receiving an INVOKE event, ensuring it does not block the main
  /// thread. NOTHING: No action is required for a NOTHING event. ERROR: Display
  /// an error message for an ERROR event but do not terminate the program.
  void handleEvent(EventTy event) override {
    switch (event) {
    case EventTy::NOTHING:
      break;
    case EventTy::ERROR:
      // TODO: logging library
      std::cerr << "Event:error\n";
      break;
    case EventTy::INVOKE:
      invoke();
      break;
    default:
      // TODO: fatal error(incorrect program) or logging library
      std::cerr << "ERROR: unknown event!\n";
      break;
    }
  }

private:
  void invoke() {
    int64_t latency = 0;
    // Wrapper for latency measurement.
    // If the request is sent several times due to errors, the delay is
    // considered only for the last attempt.
    auto &&get_balance_wrapper = [&]() {
      auto startTime = std::chrono::high_resolution_clock::now();
      auto &&response = m_client.getBalance(m_pubkey);
      auto endTime = std::chrono::high_resolution_clock::now();
      latency = std::chrono::duration_cast<std::chrono::milliseconds>(endTime -
                                                                      startTime)
                    .count();
      return response;
    };

    // reduce responses with 429 code
    m_lr_controller.wait_limit_rate();

    // 5 attempts is maximum
    HTTPErrorHandler error_handler(5);
    auto &&response = error_handler.invoke(get_balance_wrapper);

    if (cpr::status::is_success(response.status_code)) {
      char *ct;
      rapidjson::Document document;
      if (document.ParseInsitu(response.text.data()).HasParseError())
        throw std::runtime_error("Parsing error");
      if (document.HasMember("result") &&
          document["result"].HasMember("value") &&
          document["result"].HasMember("context") &&
          document["result"]["context"].HasMember("slot")) {
        m_result_container.emplace_back(
            document["result"]["context"]["slot"].GetUint64(),
            document["result"]["value"].GetUint64(), latency);
      } else {
        // TODO: logging library
        std::cerr << "Invoke error:Incomplete response: " << response.text
                  << std::endl;
      }
    } else {
      // TODO: logging library
      std::cerr << "Invoke error: " << response.status_code << ": "
                << response.text << std::endl;
    }
  }
};
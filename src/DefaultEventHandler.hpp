#pragma once

#include "Container.hpp"
#include "IEventHandler.hpp"
#include "SolanaAPI.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <chrono>
#include <cstddef>
#include <stdexcept>

class DefaultEventHandler final : public IEventHandler {
  SolanaRPCClient m_client;
  std::string m_pubkey;
  // FIXME: it is bad practice to save reference in a class.
  ConcurrentContainer<size_t, size_t> &m_result_container;

public:
  DefaultEventHandler(std::string endpoint, std::string pubkey,
                      ConcurrentContainer<size_t, size_t> &res_container)
      : m_client(std::move(endpoint)), m_pubkey(std::move(pubkey)),
        m_result_container(res_container) {}

  void handleEvent(EventTy event) override {
    switch (event) {
    case EventTy::NOTHING:
      break;
    case EventTy::ERROR:
      // TODO: logging library
      std::cerr << "Error: " << std::endl;
      break;
    case EventTy::INVOKE:
      invoke();
      break;
    default:
      // TODO: fatal error(incorrect program) or logging library
      std::cerr << "ERROR: unknown event!" << std::endl;
      break;
    }
  }

  void invoke() {
    auto startTime = std::chrono::high_resolution_clock::now();
    auto &&response = m_client.getBalance(m_pubkey);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime).count();

    // TODO: error handling and place result to the container
    if (cpr::status::is_success(response.status_code)) {
      char *ct;
      rapidjson::Document document;
      if (document.ParseInsitu(response.text.data()).HasParseError())
        throw std::runtime_error("Parsing error");
      if (document.HasMember("result") &&
          document["result"].HasMember("value") &&
          document["result"].HasMember("context") &&
          document["result"]["context"].HasMember("slot")) {
        // balance in: document["result"]["value"].GetUint64()
        m_result_container.emplace_back(
            document["result"]["context"]["slot"].GetUint64(), latency);
      } else {
        std::cerr << "Invoke error:Incomplete response: " << response.text
                  << std::endl;
      }
    } else {
      std::cerr << "Invoke error: " << response.status_code << std::endl;
    }
  }
};
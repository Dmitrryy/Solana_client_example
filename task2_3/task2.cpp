#include "Container.hpp"
#include "DefaultEventHandler.hpp"
#include <SolanaAPI.hpp>

#include <iostream>
#include <thread>
#include <vector>

int main() {
  size_t num_tasks = 1000;
  std::vector<EventTy> m_events(num_tasks);
  for (size_t i = 0; i < num_tasks; ++i) {
    m_events[i] = static_cast<EventTy>(i % 3);
  }

  // <slot, latency>
  ConcurrentContainer<size_t, size_t> results;

  {
    // TODO: use thread pool with fixed number of threads.
    // TODO: We can avoid unnecessary requests, which will obviously 
    //       lead to error 429. To do this, you need to get a rate limit 
    //       from the server and send requests no more often than this limit.
    std::vector<std::jthread> threads;
    threads.reserve(num_tasks);
    for (size_t i = 0; i < num_tasks; ++i) {
      auto cur_event = m_events[i];
      threads.emplace_back(std::jthread([&, event = cur_event]() {
        DefaultEventHandler handler(
            // "https://api.testnet.solana.com/",
            "https://api.devnet.solana.com/",
            "CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM", results);
        handler.handleEvent(event);
      }));
    }
  }

  // hear all tasks must be completed
  std::cout << "Results count: " << results.size() << std::endl;
  std::cout << "Standard deviation: " << results.standard_deviation()
            << std::endl;

  return 0;
}
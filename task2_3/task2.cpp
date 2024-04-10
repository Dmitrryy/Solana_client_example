#include "Container.hpp"
#include "DefaultEventHandler.hpp"

#include <tbb/task_group.h>

#include <iostream>
#include <vector>

// FIXME: container and rateController shouldn't be global.
//        (thread_local doesn't work inside functions).
// <slot, latency>
// Count standard deviation in last 10 slots
ConcurrentContainer<size_t, size_t> results(10);
// NOTE: 50 requests for testnet
LimitRateController limit_controller(10000, 200);
// OPTIMIZATION: create client for each thread only once
thread_local DefaultEventHandler
    event_handler("https://api.devnet.solana.com/",
                  "CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM", results,
                  limit_controller);

int main() {
  // generate syntactic events stream
  size_t num_tasks = 1000;
  std::vector<EventTy> m_events(num_tasks);
  for (size_t i = 0; i < num_tasks; ++i) {
    m_events[i] = static_cast<EventTy>(i % 3);
  }

  // precess events in parallel.
  // TBB organizes a queue of tasks with a pool of threads, the number of which
  // can be adjusted.
  tbb::task_group tg;
  for (size_t i = 0; i < num_tasks; ++i) {
    auto cur_event = m_events[i];
    tg.run([cur_event]() { event_handler.handleEvent(cur_event); });
  }
  tg.wait();

  // hear all tasks must be completed
  std::cout << "Results count: " << results.size() << std::endl;
  std::cout << "Newest balance: " << std::get<2>(results.top_older())
            << std::endl;
  std::cout << "Oldest slot: " << std::get<0>(results.top_older()) << std::endl;
  std::cout << "Newest slot: " << std::get<0>(results.top_newer()) << std::endl;
  std::cout << "Standard deviation: " << results.standard_deviation() << " ms"
            << std::endl;

  return 0;
}
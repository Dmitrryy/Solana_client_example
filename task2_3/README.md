# Task 2

This assignment builds upon the work completed in Task 1. 
Your objective is to implement a 
* GET method whose response includes the "slot" field, serving as a temporal marker within the Solana ecosystem.
* Implement an event handler to process incoming events in parallel. The types of events are INVOKE, NOTHING, and ERROR:
    * INVOKE: Execute the GET method implemented in Point 1 in the background upon receiving an INVOKE event, ensuring it does not block the main thread.
    * NOTHING: No action is required for a NOTHING event.
    * ERROR: Display an error message for an ERROR event but do not terminate the program.
* Store the results of the GET method in a common container of your choice (or one you've implemented), allowing the main thread to retrieve the oldest result in O(1) time complexity. The results should be sorted by the timestamp specified in the HTTP response and include the latency of the request.

## Solution

General architecture: the main thread generates tasks, thread pool solve them and put the results in a container.

The idea of container sorting: the sorting key has locality in time, that is, the inserted element is most likely to be at the end of the container. Under this assumption, the insertion will take O(P), where P is the number of threads in the program.

# Task 3

Our task is to enhance the functionality of the program in Task 2 (container) to support real-time tracking of the standard deviation of request latencies. This tracking should cover all GET requests made within a specified time window T, starting from the latest response timestamp X and extending backwards to X−T. The Goal is  to have fast queries for this statistics.

## Solution

Since the array of results is sorted, each time a new element is inserted, you can update the window [X - T, X] for O(1) in total.

Before determining how to update the result in the window, convert the formula:
$$ \sigma = \sqrt{\frac{\sum_{i=1}^{n}(x_i - \mu)^2}{n}} $$
$$ \sigma = \sqrt{\frac{\sum_{i=1}^{n}x_i^2 - 2\mu \sum_{i=1}^{n}x_i + n\mu^2}{n}} $$
$$ \sigma = \sqrt{\frac{\sum_{i=1}^{n}x_i^2}{n} - \frac{2\mu \sum_{i=1}^{n}x_i}{n} + \mu^2} $$
$$ \sigma = \sqrt{\frac{\sum_{i=1}^{n}x_i^2}{n} - \mu^2} $$

It can be seen from the formula that 3 quantities are needed to calculate $\sigma$: sum of squares, sum, count (N).

All 3 numbers are updated in O(1) when an element is inserted and allow you to calculate the standard deviation in O(1).

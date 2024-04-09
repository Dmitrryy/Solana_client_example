# Task 1

In this assignment, your goal is to implement one or more GET methods from the Solana RPC API using C++. You can familiarize yourself with general information about the Solana blockchain here. The complete list of HTTP methods for RPCs can be found here.
You may choose any libraries you find appropriate for this task, and you have the freedom to design the architecture of your program as you see fit. This task is an opportunity to showcase your coding style, ethics, and your approach to writing clean and scalable code.
Submission Requirements:

Your submission should include the source code and a README file. The README should outline your design and architecture choices, explain which libraries you chose and why, and provide instructions on how to build and run your code.


## Choosing a library:
Libraries are searched according to the following parameters: performance, dependencies, ease of use. Since the task is aimed at interacting with cryptocurrency wallets, the performance comes first.

Measurements of the 3 most popular libraries were carried out: `cURL`, `cpp-httplib`, `cpprest`. Environment: locally running `solana-test-validator`, core i7 gen 11, Ubuntu-22 (WSL), clang-17.

Test: in the loop of 100000 iterations - building a json query string for the Solana `getBalance` method, sending a request to the server, processing the json response (extracting the balance value and saving it).

| Benchmark               | Request Creation (ns) | Request Latency (ns)  | JSON Extraction (ns) | Balance(lamports) |
|-------------------------|-----------------------|-----------------------|----------------------|-------------------|
| cpp-httplib + rapidjson | 1011                  | 112802                | 868                  | 32000000000       |
| libcurl + rapidjson     | 948                   | 95761                 | 868                  | 32000000000       |
| cpprest                 | 5269                  | 263520                | 130960               | 32000000000       |

A short explanation of the results: In `cURL` and `cpp-httplib`, the request is blocked. While requests in cpprest work asynchronously, creating a pool of threads (about 40) working on the task queue. A task is also created to get a json file from the response: The parsing of the string into json takes of `1 mcs`, that is, `130 - 1 = 129mcs` is the approximate delay time in the task queue. Approximately the same delay is observed in the delay in receiving a response to a request compared to `cpp-httplib`. `cpp-httplib` is slower that `libcurl` by around `15%`.

Result: in the task, the library only needs to send and receive HTTP requests with high performance. I choose `cURL`. Why not `cURLcpp`?: This is a library sponsored by the open community, from which we can conclude that the project has not been tested as much as `cURL`. Moreover, a self-written function-wrapper for sending a request will be enough for a simple task.

Update: The benchmarking described above does not quite correctly describe the real conditions. In fact, the delay of an Internet request is measured in ~`ms`. (It is worth noting that even with such delays, `cpprest` lags ~ 15% behind `cURL`).
By this point, I had already encountered difficulties inventing bicycles on `cURL` (for example, parsing the HTTP header to get information about rate limits). The library selection has been revised in favor of `cpr` (C++ Requests: Curl for People). In benchmarks for access to a remote server, `cpr` shows the performance as in `cURL`.

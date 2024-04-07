#include <chrono>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <cpprest/http_client.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::json;

constexpr auto ENDPOINT = "http://127.0.0.1:8899";
constexpr auto PUBKEY = "CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM";

void cpprest_bench(size_t N) {
  std::cout << "-----------------------------=cpprest bench "
               "start=-----------------------------\n";

  std::vector<int64_t> request_create_times(N);
  std::vector<int64_t> request_latency_times(N);
  std::vector<int64_t> js_extr_times(N);

  http_client client(ENDPOINT);

  for (size_t i = 0; i < N; ++i) {
    auto requestCreateStartTime = std::chrono::high_resolution_clock::now();
    value requestBody;
    requestBody[U("jsonrpc")] = value::string(U("2.0"));
    requestBody[U("id")] = value::string(U("1"));
    requestBody[U("method")] = value::string(U("getBalance"));
    requestBody[U("params")] = value::array(1);
    requestBody[U("params")].as_array().at(0) = value::string(PUBKEY);

    http_request request(methods::POST);
    request.set_body(requestBody);
    request.headers()[U("Content-Type")] = U("application/json");

    // Latency definition:
    // Amount of time between request is made by the user to the time it takes
    // for the response to get back to that user.
    auto RequestStartTime = std::chrono::high_resolution_clock::now();
    decltype(RequestStartTime) RequestReadyTime;

    // Send the request asynchronously
    client.request(request)
        .then([&](http_response response) {
          RequestReadyTime = std::chrono::high_resolution_clock::now();
          auto &&responseBody = response.extract_json().get();
          responseBody.has_field(U("result"));
          auto balance = responseBody[U("result")][U("value")].as_number();
        })
        .wait();
    auto BalanceReadyTime = std::chrono::high_resolution_clock::now();

    using TimerResolution = std::chrono::nanoseconds;
    const auto request_creation_time =
        std::chrono::duration_cast<TimerResolution>(RequestStartTime -
                                                    requestCreateStartTime)
            .count();
    request_create_times[i] = request_creation_time;
    const auto request_latency_time =
        std::chrono::duration_cast<TimerResolution>(RequestReadyTime -
                                                    RequestStartTime)
            .count();
    request_latency_times[i] = request_latency_time;
    const auto json_extraction_time =
        std::chrono::duration_cast<TimerResolution>(BalanceReadyTime -
                                                    RequestReadyTime)
            .count();
    js_extr_times[i] = json_extraction_time;
  }

  // calculate means
  std::cout << "Request creation: "
            << std::accumulate(request_create_times.begin(),
                               request_create_times.end(), 0LL) /
                   N
            << " ns\n";
  std::cout << "Request Latency: "
            << std::accumulate(request_latency_times.begin(),
                               request_latency_times.end(), 0LL) /
                   N
            << " ns\n";
  std::cout << "Json extraction: "
            << std::accumulate(js_extr_times.begin(), js_extr_times.end(),
                               0LL) /
                   N
            << " ns\n";
}

#include <curl/curl.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace {
std::size_t callback(const char *in, std::size_t size, std::size_t num,
                     std::string *out) {
  const std::size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}
} // namespace

void libcurl_test(size_t N) {
  std::cout << "-----------------------------=libcurl bench "
               "start=-----------------------------\n";
  std::vector<int64_t> request_create_times(N);
  std::vector<int64_t> request_latency_times(N);
  std::vector<int64_t> js_extr_times(N);
  std::vector<int64_t> balances(N);

  CURLcode res;
  CURL *hnd;
  struct curl_slist *slist1;
  slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Content-Type: application/json");

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_URL, ENDPOINT);
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 10);

  for (size_t i = 0; i < N; ++i) {

    auto requestCreateStartTime = std::chrono::high_resolution_clock::now();
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

    document.AddMember("id", "1", allocator);
    document.AddMember("jsonrpc", "2.0", allocator);
    document.AddMember("method", "getBalance", allocator);
    document.AddMember("params", rapidjson::Value(rapidjson::kArrayType),
                       allocator);
    document["params"].PushBack("CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM",
                                allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, sb.GetString());

    auto RequestStartTime = std::chrono::high_resolution_clock::now();

    // Response information.
    std::unique_ptr<std::string> httpData(new std::string());

    // Hook up data handling function.
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, callback);

    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, httpData.get());

    res = curl_easy_perform(hnd);
    auto RequestReadyTime = std::chrono::high_resolution_clock::now();

    if (CURLE_OK == res) {
      char *ct;
      res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &ct);
      if ((CURLE_OK == res) && ct) {
        if (document.ParseInsitu(httpData->data()).HasParseError())
          throw std::runtime_error("Parsing error");
        auto balance = document[U("result")][U("value")].GetUint64();
        balances[i] = balance;
      }
    }
    auto BalanceReadyTime = std::chrono::high_resolution_clock::now();

    using TimerResolution = std::chrono::nanoseconds;
    const auto request_creation_time =
        std::chrono::duration_cast<TimerResolution>(RequestStartTime -
                                                    requestCreateStartTime)
            .count();
    request_create_times[i] = request_creation_time;
    const auto request_latency_time =
        std::chrono::duration_cast<TimerResolution>(RequestReadyTime -
                                                    RequestStartTime)
            .count();
    request_latency_times[i] = request_latency_time;
    const auto json_extraction_time =
        std::chrono::duration_cast<TimerResolution>(BalanceReadyTime -
                                                    RequestReadyTime)
            .count();
    js_extr_times[i] = json_extraction_time;
  }

  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(slist1);
  slist1 = NULL;

  // calculate means
  std::cout << "Request creation: "
            << std::accumulate(request_create_times.begin(),
                               request_create_times.end(), 0LL) /
                   N
            << " ns\n";
  std::cout << "Request Latency: "
            << std::accumulate(request_latency_times.begin(),
                               request_latency_times.end(), 0LL) /
                   N
            << " ns\n";
  std::cout << "Json extraction: "
            << std::accumulate(js_extr_times.begin(), js_extr_times.end(),
                               0LL) /
                   N
            << " ns\n";
  std::cout << "Balance: " << balances.back() << std::endl;
}

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

void cpp_httplib(size_t N) {
  std::cout << "-----------------------------=cpp httplib bench "
               "start=-----------------------------\n";
  std::vector<int64_t> request_create_times(N);
  std::vector<int64_t> request_latency_times(N);
  std::vector<int64_t> js_extr_times(N);
  std::vector<int64_t> balances(N);
  httplib::Client cli(ENDPOINT);

  const std::string data_type = "application/json";
  const std::string path = "/api/data";
  const httplib::Headers headers = {{"Content-Type", "application/json"}};

  for (size_t i = 0; i < N; ++i) {
    auto requestCreateStartTime = std::chrono::high_resolution_clock::now();

    // Prepare JSON data
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

    document.AddMember("id", "1", allocator);
    document.AddMember("jsonrpc", "2.0", allocator);
    document.AddMember("method", "getBalance", allocator);
    document.AddMember("params", rapidjson::Value(rapidjson::kArrayType),
                       allocator);
    document["params"].PushBack("CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM",
                                allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);

    // Set headers for JSON content

    // Send the POST request with JSON payload

    auto RequestStartTime = std::chrono::high_resolution_clock::now();
    const std::string req_body(sb.GetString());
    auto res = cli.Post(path, req_body, data_type);
    auto RequestReadyTime = std::chrono::high_resolution_clock::now();

    // Check the response
    if (res && res->status == 200) {
      // Parse the JSON response
      if (document.ParseInsitu(res->body.data()).HasParseError())
        throw std::runtime_error("Parsing error");
      auto balance = document[U("result")][U("value")].GetUint64();
      balances[i] = balance;
    } else {
      throw std::runtime_error("Request failed");
    }
    auto BalanceReadyTime = std::chrono::high_resolution_clock::now();

    using TimerResolution = std::chrono::nanoseconds;
    const auto request_creation_time =
        std::chrono::duration_cast<TimerResolution>(RequestStartTime -
                                                    requestCreateStartTime)
            .count();
    request_create_times[i] = request_creation_time;
    const auto request_latency_time =
        std::chrono::duration_cast<TimerResolution>(RequestReadyTime -
                                                    RequestStartTime)
            .count();
    request_latency_times[i] = request_latency_time;
    const auto json_extraction_time =
        std::chrono::duration_cast<TimerResolution>(BalanceReadyTime -
                                                    RequestReadyTime)
            .count();
    js_extr_times[i] = json_extraction_time;
  }

  // calculate means
  std::cout << "Request creation: "
            << std::accumulate(request_create_times.begin(),
                               request_create_times.end(), 0LL) /
                   N
            << " ns\n";
  std::cout << "Request Latency: "
            << std::accumulate(request_latency_times.begin(),
                               request_latency_times.end(), 0LL) /
                   N
            << " ns\n";
  std::cout << "Json extraction: "
            << std::accumulate(js_extr_times.begin(), js_extr_times.end(),
                               0LL) /
                   N
            << " ns\n";
  std::cout << "Balance: " << balances.back() << std::endl;
}

int main() {
  // disable unroll optimizations
  volatile size_t N = 100000;
  //   volatile size_t N = 1;

  cpp_httplib(N);
  libcurl_test(N);
  cpprest_bench(N);
}
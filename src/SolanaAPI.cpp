#include "SolanaAPI.hpp"

#include <chrono>
#include <memory>

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

SolanaRPCClient::SolanaRPCClient(const std::string &endpoint)
    : m_rpcEndpoint(endpoint) {}

uint64_t SolanaRPCClient::getBalance(const std::string &pubkey) {
  CURLcode res;
  CURL *hnd;
  struct curl_slist *slist1;
  slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Content-Type: application/json");

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_URL, m_rpcEndpoint.c_str());
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 10);

  rapidjson::Document document;
  document.SetObject();
  rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

  document.AddMember("id", "1", allocator);
  document.AddMember("jsonrpc", "2.0", allocator);
  document.AddMember("method", "getBalance", allocator);
  document.AddMember("params", rapidjson::Value(rapidjson::kArrayType),
                     allocator);
  document["params"].PushBack(rapidjson::StringRef(pubkey.data()), allocator);

  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  document.Accept(writer);
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, sb.GetString());

  // Response information.
  std::unique_ptr<std::string> httpData(new std::string());

  // Hook up data handling function.
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, callback);

  // Hook up data container (will be passed as the last parameter to the
  // callback handling function).  Can be any pointer type, since it will
  // internally be passed as a void pointer.
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, httpData.get());

  auto RequestStartTime = std::chrono::high_resolution_clock::now();
  res = curl_easy_perform(hnd);
  auto RequestReadyTime = std::chrono::high_resolution_clock::now();

  if (CURLE_OK == res) {
    char *ct;
    res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &ct);
    if ((CURLE_OK == res) && ct) {
      if (document.ParseInsitu(httpData->data()).HasParseError())
        throw std::runtime_error("Parsing error");
      return document["result"]["value"].GetUint64();
    }
  }

  // FIXME: won't be executed after exception
  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(slist1);
  slist1 = NULL;

  throw std::runtime_error("Request error");
  return {};
}

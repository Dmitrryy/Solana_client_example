#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "curl/curl.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class SolanaCurlRequestError : public std::runtime_error {
public:
  SolanaCurlRequestError(const std::string &message, CURLcode errorCode)
      : std::runtime_error(message), error_code(errorCode) {}

  auto errorCode() const { return error_code; }

private:
  CURLcode error_code;
};

class SolanaCurlRequest final {
  std::string m_rpcEndpoint;
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> m_curl_handler;
  std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)>
      m_httpHeader;

public:
  SolanaCurlRequest(std::string endpoint)
      : m_rpcEndpoint(std::move(endpoint)),
        m_curl_handler(curl_easy_init(), curl_easy_cleanup),
        m_httpHeader(
            curl_slist_append(nullptr, "Content-Type: application/json"),
            curl_slist_free_all) {
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_URL, m_rpcEndpoint.c_str());
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_HTTPHEADER,
                     m_httpHeader.get());
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_WRITEFUNCTION,
                     _bodyCallback);
  }

  void SetUrl(const std::string &url) {
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_URL, url.c_str());
  }

  void SetTimeout(int timeout_secs) {
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_TIMEOUT, timeout_secs);
  }

  void SetFollowLocation(bool follow) {
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_FOLLOWLOCATION,
                     follow ? 1L : 0L);
  }

  template <CURLINFO InfoTy> const char *getInfo() const {
    const char *res = nullptr;
    auto retCode = curl_easy_getinfo(m_curl_handler.get(), InfoTy, &res);
    if (retCode != CURLE_OK) {
      throw SolanaCurlRequestError(curl_easy_strerror(retCode), retCode);
    }
    return res;
  }

  std::pair<std::string, std::string> request(const std::string &body) {
    std::string response, headers;
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_curl_handler.get(), CURLOPT_HEADERDATA, &headers);

    auto res = curl_easy_perform(m_curl_handler.get());

    if (res != CURLE_OK) {
      throw SolanaCurlRequestError(curl_easy_strerror(res), res);
    }
    return {response, headers};
  }

private:
  static size_t _bodyCallback(char *ptr, size_t size, size_t nmemb,
                              void *userdata) {
    const size_t real_size = size * nmemb;
    std::string *response = static_cast<std::string *>(userdata);
    response->append(ptr, real_size);
    return real_size;
  }

  static size_t _headerCallback(char *ptr, size_t size, size_t nmemb,
                                void *userdata) {
    const size_t real_size = size * nmemb;
    std::string *response = static_cast<std::string *>(userdata);
    response->append(ptr, real_size);
    return real_size;
  }
};

// A class to encapsulate RPC calls might be useful
class SolanaRPCClient {
public:
  SolanaRPCClient(std::string endpoint) : m_requester(std::move(endpoint)) {}

  uint64_t getBalance(const std::string &pubkey) {
    rapidjson::Document JSONrequest;
    JSONrequest.SetObject();
    rapidjson::Document::AllocatorType &allocator = JSONrequest.GetAllocator();

    // prepare json request
    JSONrequest.AddMember("id", "1", allocator);
    JSONrequest.AddMember("jsonrpc", "2.0", allocator);
    JSONrequest.AddMember("method", "getBalance", allocator);
    JSONrequest.AddMember("params", rapidjson::Value(rapidjson::kArrayType),
                          allocator);
    JSONrequest["params"].PushBack(rapidjson::StringRef(pubkey.data()),
                                   allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    JSONrequest.Accept(writer);

    auto &&[responseStr, headersStr] = m_requester.request(sb.GetString());
    // check that content is json
    const std::string responseFormat =
        m_requester.getInfo<CURLINFO_CONTENT_TYPE>();

    // TODO: check: charset=utf-8
    if (responseFormat.rfind("application/json", 0) != 0) {
      throw std::runtime_error("Unsupported response format: " +
                               responseFormat);
    }

    rapidjson::Document JSONresponse;
    // FIXME: we shouldn't fall in this case
    if (JSONresponse.ParseInsitu(responseStr.data()).HasParseError())
      throw std::runtime_error("Parsing error");

    if (JSONresponse.HasMember("result") &&
        JSONresponse["result"].HasMember("value")) {
      return JSONresponse["result"]["value"].GetUint64();
    } else {
      std::cerr << "Incomplete response: " << responseStr << std::endl;
      std::cerr << "    Header: " << headersStr << std::endl;
    }
    // FIXME: magic value after error???
    return -1;
  }

private:
private:
  SolanaCurlRequest m_requester;
};

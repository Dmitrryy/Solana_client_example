#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "cpr/response.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <cpr/cpr.h>

// An light wrapper for making requests to the Solana HTTP methods.
//
// NOTE: The object is not secure in terms of multithreading!
// Each thread must have its own instance of the class.
class SolanaRPCClient {
public:
  SolanaRPCClient(std::string endpoint) {
    m_session.SetUrl(cpr::Url{endpoint});
    m_session.SetHeader(cpr::Header{{"Content-Type", "application/json"}});
    m_session.SetTimeout(20000);

    // init json template
    m_requestTmp.SetObject();
    rapidjson::Document::AllocatorType &allocator = m_requestTmp.GetAllocator();
    m_requestTmp.AddMember("id", "1", allocator);
    m_requestTmp.AddMember("jsonrpc", "2.0", allocator);
    m_requestTmp.AddMember("method", "INVALID_METHOD", allocator);
    m_requestTmp.AddMember("params", rapidjson::Value(rapidjson::kArrayType),
                           allocator);
  }

  // The method returns a raw response from the server, why? Processing the
  // response from the server is a separate responsibility that may depend on
  // the usage scenario (for example, wait 10 seconds to send a repeat after 429
  // or not).
  // FIXME: replace rpc::Response with a custom type that would hide the
  // implementation detail of the class - the use of the cpr library.
  cpr::Response getBalance(const std::string &pubkey) {
    // prepare json request
    m_requestTmp["method"].SetString("getBalance");
    auto &&arr = m_requestTmp["params"].GetArray();
    arr.Clear();
    arr.PushBack(rapidjson::StringRef(pubkey.data()),
                 m_requestTmp.GetAllocator());

    m_session.SetBody(jsonToStr(m_requestTmp));
    return m_session.Post();
  }

private:
  static std::string jsonToStr(const rapidjson::Document &doc) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    doc.Accept(writer);
    return {sb.GetString(), sb.GetSize()};
  }

private:
  cpr::Session m_session;
  rapidjson::Document m_requestTmp;
};

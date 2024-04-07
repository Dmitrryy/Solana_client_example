#pragma once

#include <string>

// A class to encapsulate RPC calls might be useful
class SolanaRPCClient {
public:
    SolanaRPCClient(const std::string& endpoint);

    uint64_t getBalance(const std::string& pubkey); 
private:
    

private:
    std::string m_rpcEndpoint;
};


#include <SolanaAPI.hpp>

#include <iostream>

int main() {
    SolanaRPCClient cl("https://api.testnet.solana.com/");
    auto &&res = cl.getBalance("CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM");
    std::cout << "Balance: " << res << std::endl;

    return 0;
}
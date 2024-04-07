#include <SolanaAPI.hpp>

#include <iostream>

int main() {
    SolanaRPCClient cl("http://127.0.0.1:8899");
    auto &&res = cl.getBalance("CsobwrE9x7qfKC23GFWPq8FMVWzVCErWh1A7C2dMBNMM");
    std::cout << "Balance: " << res << std::endl;

    return 0;
}
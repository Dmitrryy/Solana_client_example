# Interview_Solana

## Solana CLI install
https://solana.com/developers/guides/getstarted/setup-local-development

### Linux
```
sudo apt-get install -y \
    build-essential \
    pkg-config \
    libudev-dev llvm libclang-dev \
    protobuf-compiler libssl-dev
```

```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
```

```
sh -c "$(curl -sSfL https://release.solana.com/stable/install)"
```
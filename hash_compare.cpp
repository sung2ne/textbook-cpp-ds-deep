// filename: hash_compare.cpp
// g++ -std=c++17 -O2 -o hash_compare hash_compare.cpp
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

// FNV-1a 64비트 구현
uint64_t fnv1a_64(std::string_view data) {
    constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME  = 1099511628211ULL;

    uint64_t hash = FNV_OFFSET;
    for (unsigned char c : data) {
        hash ^= static_cast<uint64_t>(c);
        hash *= FNV_PRIME;
    }
    return hash;
}

// MurmurHash3 (32비트 간략 버전)
uint32_t murmur3_32(std::string_view key, uint32_t seed = 0) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
    const int      nblocks = static_cast<int>(key.size()) / 4;

    uint32_t h1 = seed;
    constexpr uint32_t c1 = 0xcc9e2d51;
    constexpr uint32_t c2 = 0x1b873593;

    const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data);
    for (int i = 0; i < nblocks; ++i) {
        uint32_t k1 = blocks[i];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);  // ROTL32
        k1 *= c2;
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);  // ROTL32
        h1 = h1 * 5 + 0xe6546b64;
    }

    const uint8_t* tail = data + nblocks * 4;
    uint32_t k1 = 0;
    switch (key.size() & 3) {
        case 3: k1 ^= static_cast<uint32_t>(tail[2]) << 16; [[fallthrough]];
        case 2: k1 ^= static_cast<uint32_t>(tail[1]) << 8;  [[fallthrough]];
        case 1: k1 ^= static_cast<uint32_t>(tail[0]);
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
    }

    h1 ^= static_cast<uint32_t>(key.size());
    // fmix32
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

// xxHash32 간략 구현 (단일 블록 처리)
uint32_t xxhash32(std::string_view key, uint32_t seed = 0) {
    constexpr uint32_t PRIME1 = 2654435761U;
    constexpr uint32_t PRIME2 = 2246822519U;
    constexpr uint32_t PRIME3 = 3266489917U;
    constexpr uint32_t PRIME4 =  668265263U;
    constexpr uint32_t PRIME5 =  374761393U;

    const uint8_t* p   = reinterpret_cast<const uint8_t*>(key.data());
    const uint8_t* end = p + key.size();
    uint32_t h32;

    if (key.size() >= 16) {
        uint32_t v1 = seed + PRIME1 + PRIME2;
        uint32_t v2 = seed + PRIME2;
        uint32_t v3 = seed;
        uint32_t v4 = seed - PRIME1;
        while (p <= end - 16) {
            auto round = [](uint32_t v, uint32_t i) {
                v += i * PRIME2;
                v = (v << 13) | (v >> 19);
                return v * PRIME1;
            };
            uint32_t block[4];
            __builtin_memcpy(block, p, 16);
            v1 = round(v1, block[0]);
            v2 = round(v2, block[1]);
            v3 = round(v3, block[2]);
            v4 = round(v4, block[3]);
            p += 16;
        }
        h32 = ((v1 << 1) | (v1 >> 31))
            + ((v2 << 7) | (v2 >> 25))
            + ((v3 << 12) | (v3 >> 20))
            + ((v4 << 18) | (v4 >> 14));
    } else {
        h32 = seed + PRIME5;
    }
    h32 += static_cast<uint32_t>(key.size());

    while (p <= end - 4) {
        uint32_t block; __builtin_memcpy(&block, p, 4);
        h32 += block * PRIME3;
        h32 = ((h32 << 17) | (h32 >> 15)) * PRIME4;
        p += 4;
    }
    while (p < end) {
        h32 += (*p) * PRIME5;
        h32 = ((h32 << 11) | (h32 >> 21)) * PRIME1;
        ++p;
    }
    h32 ^= h32 >> 15;
    h32 *= 0x85ebca77;
    h32 ^= h32 >> 13;
    h32 *= 0xc2b2ae3dU;
    h32 ^= h32 >> 16;
    return h32;
}

int main() {
    const std::string words[] = {
        "hello", "hellp", "world", "C++17", "hash_table"
    };

    std::cout << std::left;
    std::cout << "단어         | std::hash           | FNV-1a              | MurmurHash3 | xxHash32\n";
    std::cout << std::string(90, '-') << '\n';

    std::hash<std::string> std_hash;
    for (const auto& w : words) {
        std::cout << w << std::string(13 - w.size(), ' ') << "| ";
        std::cout << std_hash(w) % 1000000 << std::string(20 - std::to_string(std_hash(w) % 1000000).size(), ' ') << "| ";
        std::cout << fnv1a_64(w) % 1000000 << std::string(20 - std::to_string(fnv1a_64(w) % 1000000).size(), ' ') << "| ";
        std::cout << murmur3_32(w) % 1000000 << std::string(12 - std::to_string(murmur3_32(w) % 1000000).size(), ' ') << "| ";
        std::cout << xxhash32(w)   % 1000000 << '\n';
    }
}

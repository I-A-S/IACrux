// IACrux; The Core Library for All IA Open Source Projects
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <crux/utils.hpp>

#include <thread>

#include <vendor/wjakob/pcg32.h>

#if IA_ARCH_X64
#  include <immintrin.h>
#elif IA_ARCH_ARM64
#  include <arm_acle.h>
#endif

namespace ia::utils
{
  namespace
  {
    template<typename T> [[nodiscard]] inline auto read_unaligned(const u8 *ptr) -> T
    {
      Mut<T> v;
      std::memcpy(&v, ptr, sizeof(T));
      return v;
    }

    struct Crc32Tables
    {
      Mut<u32> table[8][256] = {};

      consteval Crc32Tables()
      {
        constexpr const u32 T = 0x82F63B78;

        for (Mut<u32> i = 0; i < 256; i++)
        {
          Mut<u32> crc = i;
          for (Mut<i32> j = 0; j < 8; j++)
          {
            crc = (crc >> 1) ^ ((crc & 1) ? T : 0);
          }
          table[0][i] = crc;
        }

        for (Mut<i32> i = 0; i < 256; i++)
        {
          for (Mut<i32> slice = 1; slice < 8; slice++)
          {
            const u32 prev = table[slice - 1][i];
            table[slice][i] = (prev >> 8) ^ table[0][prev & 0xFF];
          }
        }
      }
    };

    static constexpr const Crc32Tables CRC32_TABLES{};
  } // namespace

#if IA_ARCH_X64
  inline auto crc32_x64_hw(Ref<Span<const u8>> data) -> u32
  {
    Mut<const u8 *> p = data.data();

    Mut<u32> crc = 0xFFFFFFFF;
    Mut<usize> len = data.size();

    while (len >= 8)
    {
      const u64 chunk = read_unaligned<u64>(p);
      crc = static_cast<u32>(_mm_crc32_u64(static_cast<u64>(crc), chunk));
      p += 8;
      len -= 8;
    }

    while (len--)
    {
      crc = _mm_crc32_u8(crc, *p++);
    }

    return ~crc;
  }
#endif

#if IA_ARCH_ARM64
  __attribute__((target("+crc"))) inline auto crc32_arm64_hw(Ref<Span<const u8>> data) -> u32
  {
    Mut<const u8 *> p = data.data();

    Mut<u32> crc = 0xFFFFFFFF;
    Mut<usize> len = data.size();

    while (len >= 8)
    {
      const u64 chunk = read_unaligned<u64>(p);
      crc = __crc32cd(crc, chunk);
      p += 8;
      len -= 8;
    }

    while (len--)
    {
      crc = __crc32cb(crc, *p++);
    }

    return ~crc;
  }
#endif

  inline auto crc32_software_slice8(Ref<Span<const u8>> data) -> u32
  {
    Mut<const u8 *> p = data.data();
    Mut<u32> crc = 0xFFFFFFFF;
    Mut<usize> len = data.size();

    while (len >= 8)
    {
      const u32 term1 = crc ^ read_unaligned<u32>(p);
      const u32 term2 = read_unaligned<u32>(p + 4);

      crc = CRC32_TABLES.table[7][term1 & 0xFF] ^ CRC32_TABLES.table[6][(term1 >> 8) & 0xFF] ^
            CRC32_TABLES.table[5][(term1 >> 16) & 0xFF] ^ CRC32_TABLES.table[4][(term1 >> 24)] ^
            CRC32_TABLES.table[3][term2 & 0xFF] ^ CRC32_TABLES.table[2][(term2 >> 8) & 0xFF] ^
            CRC32_TABLES.table[1][(term2 >> 16) & 0xFF] ^ CRC32_TABLES.table[0][(term2 >> 24)];

      p += 8;
      len -= 8;
    }

    while (len--)
    {
      crc = (crc >> 8) ^ CRC32_TABLES.table[0][(crc ^ *p++) & 0xFF];
    }

    return ~crc;
  }

  auto crc32(Ref<Span<const u8>> data) -> u32
  {
#if IA_ARCH_X64
    // IACore mandates AVX2 so no need to check
    return crc32_x64_hw(data);
#elif IA_ARCH_ARM64
    if (Platform::GetCapabilities().HardwareCRC32)
    {
      return crc32_arm64_hw(data);
    }
#endif
    return crc32_software_slice8(data);
  }
} // namespace ia::utils

namespace ia::utils
{
  namespace
  {
    constexpr const u32 XXH_PRIME32_1 = 0x9E3779B1U;
    constexpr const u32 XXH_PRIME32_2 = 0x85EBCA77U;
    constexpr const u32 XXH_PRIME32_3 = 0xC2B2AE3DU;
    constexpr const u32 XXH_PRIME32_4 = 0x27D4EB2FU;
    constexpr const u32 XXH_PRIME32_5 = 0x165667B1U;

    inline auto xxh32_round(Mut<u32> seed, const u32 input) -> u32
    {
      seed += input * XXH_PRIME32_2;
      seed = std::rotl(seed, 13);
      seed *= XXH_PRIME32_1;
      return seed;
    }
  } // namespace

  auto hash_xxhash(Ref<String> string, const u32 seed) -> u32
  {
    return hash_xxhash(Span<const u8>(reinterpret_cast<const u8 *>(string.data()), string.length()), seed);
  }

  auto hash_xxhash(Ref<Span<const u8>> data, const u32 seed) -> u32
  {
    Mut<const u8 *> p = data.data();
    const u8 *const b_end = p + data.size();
    Mut<u32> h32{};

    if (data.size() >= 16)
    {
      const u8 *const limit = b_end - 16;

      Mut<u32> v1 = seed + XXH_PRIME32_1 + XXH_PRIME32_2;
      Mut<u32> v2 = seed + XXH_PRIME32_2;
      Mut<u32> v3 = seed + 0;
      Mut<u32> v4 = seed - XXH_PRIME32_1;

      do
      {
        v1 = xxh32_round(v1, read_unaligned<u32>(p));
        p += 4;
        v2 = xxh32_round(v2, read_unaligned<u32>(p));
        p += 4;
        v3 = xxh32_round(v3, read_unaligned<u32>(p));
        p += 4;
        v4 = xxh32_round(v4, read_unaligned<u32>(p));
        p += 4;
      } while (p <= limit);

      h32 = std::rotl(v1, 1) + std::rotl(v2, 7) + std::rotl(v3, 12) + std::rotl(v4, 18);
    }
    else
    {
      h32 = seed + XXH_PRIME32_5;
    }

    h32 += static_cast<u32>(data.size());

    while (p + 4 <= b_end)
    {
      const u32 t = read_unaligned<u32>(p) * XXH_PRIME32_3;
      h32 += t;
      h32 = std::rotl(h32, 17) * XXH_PRIME32_4;
      p += 4;
    }

    while (p < b_end)
    {
      h32 += (*p++) * XXH_PRIME32_5;
      h32 = std::rotl(h32, 11) * XXH_PRIME32_1;
    }

    h32 ^= h32 >> 15;
    h32 *= XXH_PRIME32_2;
    h32 ^= h32 >> 13;
    h32 *= XXH_PRIME32_3;
    h32 ^= h32 >> 16;

    return h32;
  }

  constexpr const u32 FNV1A_32_PRIME = 0x01000193;
  constexpr const u32 FNV1A_32_OFFSET = 0x811c9dc5;

  auto hash_fnv1a(Ref<String> string) -> u32
  {
    Mut<u32> hash = FNV1A_32_OFFSET;
    for (const char c : string)
    {
      hash ^= static_cast<u8>(c);
      hash *= FNV1A_32_PRIME;
    }
    return hash;
  }

  auto hash_fnv1a(Ref<Span<const u8>> data) -> u32
  {
    Mut<u32> hash = FNV1A_32_OFFSET;
    const u8 *const ptr = data.data();

    for (Mut<usize> i = 0; i < data.size(); ++i)
    {
      hash ^= ptr[i];
      hash *= FNV1A_32_PRIME;
    }
    return hash;
  }
} // namespace ia::utils

namespace ia::utils
{
  namespace
  {
    auto from_hex_char(const char c) -> i32
    {
      if (c >= '0' && c <= '9')
      {
        return c - '0';
      }
      if (c >= 'A' && c <= 'F')
      {
        return c - 'A' + 10;
      }
      if (c >= 'a' && c <= 'f')
      {
        return c - 'a' + 10;
      }
      return -1;
    }
  } // namespace

  auto get_unix_time() -> u64
  {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  }

  auto get_random() -> f32
  {
    thread_local Mut<pcg32> rng = []() {
      const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
      const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
      return pcg32(static_cast<u64>(now) ^ tid);
    }();

    return rng.nextFloat();
  }

  auto get_random(const u64 max) -> u64
  {
    thread_local Mut<pcg32> rng = []() {
      const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
      const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
      return pcg32(static_cast<u64>(now) ^ tid);
    }();

    const auto t = (static_cast<u64>(rng.nextUInt()) << 32) | rng.nextUInt();
    return t % max;
  }

  auto get_random(const i64 min, const i64 max) -> i64
  {
    return min + static_cast<i64>(get_random(max - min));
  }

  auto sleep(const u64 milliseconds) -> void
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }

  auto binary_to_hex_string(const Span<const u8> data) -> String
  {
    static constexpr const char LUT[17] = "0123456789ABCDEF";
    Mut<String> res = String();
    res.reserve(data.size() * 2);

    for (const u8 b : data)
    {
      res.push_back(LUT[(b >> 4) & 0x0F]);
      res.push_back(LUT[b & 0x0F]);
    }
    return res;
  }

  auto hex_string_to_binary(const StringView hex) -> Result<Vec<u8>>
  {
    if (hex.size() % 2 != 0)
    {
      return fail("Hex string must have even length");
    }

    Mut<Vec<u8>> out = Vec<u8>();
    out.reserve(hex.size() / 2);

    for (Mut<usize> i = 0; i < hex.size(); i += 2)
    {
      const char high = hex[i];
      const char low = hex[i + 1];

      const i32 h = from_hex_char(high);
      const i32 l = from_hex_char(low);

      if (h == -1 || l == -1)
      {
        return fail("Invalid hex character found");
      }

      out.push_back(static_cast<u8>((h << 4) | l));
    }

    return out;
  }
} // namespace ia::utils
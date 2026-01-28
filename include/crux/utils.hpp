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

#pragma once

#include <crux/crux.hpp>

#include <algorithm>

namespace ia
{
  namespace utils
  {
    auto hash_fnv1a(Ref<String> string) -> u32;
    auto hash_fnv1a(Ref<Span<const u8>> data) -> u32;

    auto hash_xxhash(Ref<String> string, const u32 seed = 0) -> u32;
    auto hash_xxhash(Ref<Span<const u8>> data, const u32 seed = 0) -> u32;

    auto crc32(Ref<Span<const u8>> data) -> u32;

    auto get_unix_time() -> u64;

    auto get_random() -> f32;
    auto get_random(const u64 max) -> u64;
    auto get_random(const i64 min, const i64 max) -> i64;

    auto sleep(const u64 milliseconds) -> void;

    auto binary_to_hex_string(const Span<const u8> data) -> String;

    auto hex_string_to_binary(const StringView hex) -> Result<Vec<u8>>;
  } // namespace utils

  namespace utils
  {
    template<typename Range> inline auto sort(ForwardRef<Range> range) -> void
    {
      std::ranges::sort(std::forward<Range>(range));
    }

    template<typename Range, typename T> inline auto binary_search_left(ForwardRef<Range> range, Ref<T> value) -> auto
    {
      return std::ranges::lower_bound(std::forward<Range>(range), value);
    }

    template<typename Range, typename T> inline auto binary_search_right(ForwardRef<Range> range, Ref<T> value) -> auto
    {
      return std::ranges::upper_bound(std::forward<Range>(range), value);
    }

    template<typename T> inline auto hash_combine(MutRef<u64> seed, Ref<T> v) -> void
    {
      Mut<u64> h = 0;

      if constexpr (std::is_constructible_v<StringView, T>)
      {
        const StringView sv(v);
        const ankerl::unordered_dense::hash<StringView> hasher;
        h = hasher(sv);
      }
      else
      {
        const ankerl::unordered_dense::hash<T> hasher;
        h = hasher(v);
      }

      seed ^= h + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
    }

    template<typename... Args> inline auto compute_hash(Ref<Args>... args) -> u64
    {
      Mut<u64> seed = 0;
      (hash_combine(seed, args), ...);
      return seed;
    }

    template<typename T, typename... MemberPtrs>
    inline auto compute_hash_flat(Ref<T> obj, const MemberPtrs... members) -> u64
    {
      Mut<u64> seed = 0;
      (hash_combine(seed, obj.*members), ...);
      return seed;
    }
  } // namespace utils
} // namespace ia

// -----------------------------------------------------------------------------
// MACRO: IA_MAKE_HASHABLE
//
// Injects the specialization for ankerl::unordered_dense::hash.
//
// Usage:
//   struct Vector3 { float x, y, z; };
//   IA_MAKE_HASHABLE(Vector3, &Vector3::x, &Vector3::y, &Vector3::z)
// -----------------------------------------------------------------------------
#define IA_MAKE_HASHABLE(Type, ...)                                                                                    \
  template<> struct ankerl::unordered_dense::hash<Type>                                                                \
  {                                                                                                                    \
    using is_avalanching = void;                                                                                       \
    IA_NODISCARD                                                                                                       \
    auto operator()(ia::Ref<Type> v) const noexcept -> ia::u64                                                         \
    {                                                                                                                  \
      return ia::utils::compute_hash_flat(v, __VA_ARGS__);                                                             \
    }                                                                                                                  \
  };
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

#include <iostream>

namespace ia
{
  namespace io
  {
    class IOutputStream
    {
  public:
      virtual auto put_string(StringView data) -> void = 0;
      virtual auto put_buffer(Span<const u8> data) -> void = 0;

      template<typename... Args> auto put(std::format_string<Args...> fmt, Args &&...args) -> void
      {
        put_string(std::vformat(fmt.get(), std::make_format_args(args...)));
      }

      template<usize BufferSize, typename... Args> auto put(std::format_string<Args...> fmt, Args &&...args) -> void
      {
        char buffer[BufferSize];
        auto result = std::format_to_n(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
        put_string(std::string_view(buffer, result.size));
      }
    };

    class IInputStream
    { // [IATODO]: IMPL after dinner
  public:
    };

    class StdOutStream : public IOutputStream
    {
  public:
      auto put_string(StringView data) -> void override
      {
        std::cout << data;
      };

      auto put_buffer(Span<const u8> data) -> void override
      {
        for (const auto &b : data)
          std::cout << b;
      };
    };

    class StdErrStream : public IOutputStream
    {
  public:
      auto put_string(StringView data) -> void override
      {
        std::cerr << data;
      };

      auto put_buffer(Span<const u8> data) -> void override
      {
        for (const auto &b : data)
          std::cerr << b;
      };
    };
  } // namespace io
} // namespace ia
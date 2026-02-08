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

#include <format>
#include <source_location>

namespace ia
{
  namespace logger
  {
    enum class Level : u8
    {
      Trace = 0,
      Debug,
      Info,
      Warn,
      Error,
      Fatal
    };

    using Handler = void (*)(void *user_data, Level level, StringView message, StringView file, u32 line);

    auto set_handler(Handler handler, void *user_data) -> void;

    auto dispatch(Level level, StringView message, Ref<std::source_location> loc = std::source_location::current())
        -> void;

    template<typename... Args>
    void dispatch_fmt(Level level, Ref<std::source_location> loc, std::format_string<Args...> fmt,
                      ForwardRef<Args>... args)
    {
      auto msg = std::format(fmt, std::forward<Args>(args)...);
      dispatch(level, msg, loc);
    }
  } // namespace logger
} // namespace ia

#define IA_LOG_TRACE(...)                                                                                              \
  ::ia::logger::dispatch_fmt(::ia::logger::Level::Trace, std::source_location::current(), __VA_ARGS__)
#define IA_LOG_DEBUG(...)                                                                                              \
  ::ia::logger::dispatch_fmt(::ia::logger::Level::Debug, std::source_location::current(), __VA_ARGS__)
#define IA_LOG_INFO(...)                                                                                               \
  ::ia::logger::dispatch_fmt(::ia::logger::Level::Info, std::source_location::current(), __VA_ARGS__)
#define IA_LOG_WARN(...)                                                                                               \
  ::ia::logger::dispatch_fmt(::ia::logger::Level::Warn, std::source_location::current(), __VA_ARGS__)
#define IA_LOG_ERROR(...)                                                                                              \
  ::ia::logger::dispatch_fmt(::ia::logger::Level::Error, std::source_location::current(), __VA_ARGS__)
#define IA_LOG_FATAL(...)                                                                                              \
  ::ia::logger::dispatch_fmt(::ia::logger::Level::Fatal, std::source_location::current(), __VA_ARGS__)

#if !__IA_DEBUG
#  undef IA_LOG_TRACE
#  undef IA_LOG_DEBUG
#  define IA_LOG_TRACE(...) ((void) 0)
#  define IA_LOG_DEBUG(...) ((void) 0)
#endif
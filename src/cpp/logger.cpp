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

#include <crux/logger.hpp>

#include <atomic>

namespace ia::logger
{
  namespace
  {
    std::mutex g_console_mutex;

    void default_console_handler(Level level, StringView msg, StringView file, u32 line)
    {
      auto filename = std::filesystem::path(file).filename().string();

      std::lock_guard lock(g_console_mutex);

      const char *color_code = ia::console::RESET;
      const char *level_str = "[INFO]";

      switch (level)
      {
      case Level::Trace:
        level_str = "[TRCE]";
        color_code = ia::console::CYAN;
        break;
      case Level::Debug:
        level_str = "[DEBG]";
        color_code = ia::console::MAGENTA;
        break;
      case Level::Info:
        level_str = "[INFO]";
        color_code = ia::console::GREEN;
        break;
      case Level::Warn:
        level_str = "[WARN]";
        color_code = ia::console::YELLOW;
        break;
      case Level::Error:
        level_str = "[ERRO]";
        color_code = ia::console::RED;
        break;
      case Level::Fatal:
        level_str = "[FATL]";
        color_code = ia::console::RED;
        break;
      }

      std::cout << color_code << level_str << " " << ia::console::RESET << "[" << filename << ":" << line << "] " << msg
                << "\n";
    }

    std::atomic<Handler> g_active_handler{default_console_handler};
  } // namespace

  auto set_handler(Handler handler) -> void
  {
    if (handler == nullptr)
    {
      g_active_handler.store(default_console_handler);
    }
    else
    {
      g_active_handler.store(handler);
    }
  }

  auto dispatch(Level level, StringView message, Ref<std::source_location> loc) -> void
  {
    auto handler = g_active_handler.load(std::memory_order_relaxed);
    if (handler)
    {
      handler(level, message, loc.file_name(), loc.line());
    }
  }
} // namespace ia::logger
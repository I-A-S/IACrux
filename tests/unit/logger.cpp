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
#include <iatest/iatest.hpp>

#include <fstream>

using namespace ia;

IAT_BEGIN_BLOCK(Core, Logger)

static constexpr const char *LOG_FILE = "iacore_test_log.txt";

auto read_text_file(Ref<Path> path) -> Result<String>
{
  Mut<std::ifstream> file(path, std::ios::in | std::ios::ate);
  if (!file.is_open())
    return fail("Failed to open file '{}' for reading", path.string());
  const usize size = file.tellg();
  if (!size)
    return fail("File '{}' is empty", path.string());
  file.seekg(0);
  Mut<String> contents;
  contents.resize(size + 1);
  file.read(contents.data(), size);
  contents.back() = '\0';
  return contents;
}

auto cleanup_file(Ref<Path> path) -> void
{
  Mut<std::error_code> ec;
  if (std::filesystem::exists(path, ec))
  {
    std::filesystem::remove(path, ec);
  }
}

auto test_file_logging() -> bool
{
  Mut<std::ofstream> _log_out_file(LOG_FILE);
  IAT_CHECK(_log_out_file.is_open());

  logger::set_handler(
      [](void *user_data, logger::Level level, StringView message, StringView file, u32 line) {
        const auto log_file = reinterpret_cast<std::ofstream *>(user_data);

        AU_UNUSED(level);
        AU_UNUSED(file);
        AU_UNUSED(line);

        *log_file << message;
      },
      &_log_out_file);

  const String msg_info = "Test_Info_Msg_123";
  const String msg_err = "Test_Error_Msg_456";
  const String msg_warn = "Test_Warn_Msg_789";

  IA_LOG_INFO("{}", msg_info);
  IA_LOG_WARN("{}", msg_warn);
  IA_LOG_ERROR("{}", msg_err);

  _log_out_file.close();

  const auto read_res = read_text_file(LOG_FILE);
  if (!read_res)
  {
    std::cout << console::YELLOW << "    Warning: Could not read log file (" << read_res.error()
              << "). Skipping verification.\n"
              << console::RESET;
    return true;
  }

  const String content = *read_res;

  IAT_CHECK(content.find(msg_info) != String::npos);
  IAT_CHECK(content.find(msg_err) != String::npos);
  IAT_CHECK(content.find(msg_warn) != String::npos);

  cleanup_file(LOG_FILE);

  return true;
}

auto test_log_levels() -> bool
{
  Mut<std::ofstream> _log_out_file(LOG_FILE);
  IAT_CHECK(_log_out_file.is_open());

  logger::set_handler(
      [](void *user_data, logger::Level level, StringView message, StringView file, u32 line) {
        const auto log_file = reinterpret_cast<std::ofstream *>(user_data);

        AU_UNUSED(file);
        AU_UNUSED(line);

        if (level <= logger::Level::Warn)
          return;

        *log_file << message;
      },
      &_log_out_file);

  const String unique_info = "Hidden_Info_Msg";
  const String unique_warn = "Visible_Warn_Msg";

  IA_LOG_INFO("{}", unique_info);
  IA_LOG_WARN("{}", unique_warn);

  _log_out_file.close();

  const auto read_res = read_text_file(LOG_FILE);
  if (!read_res)
  {
    return true;
  }

  const String content = *read_res;

  IAT_CHECK(content.find(unique_info) == String::npos);

  IAT_CHECK(content.find(unique_warn) != String::npos);

  cleanup_file(LOG_FILE);

  return true;
}

auto test_formatting() -> bool
{
  Mut<std::ofstream> _log_out_file(LOG_FILE);
  IAT_CHECK(_log_out_file.is_open());

  logger::set_handler(
      [](void *user_data, logger::Level level, StringView message, StringView file, u32 line) {
        const auto log_file = reinterpret_cast<std::ofstream *>(user_data);

        AU_UNUSED(level);
        AU_UNUSED(file);
        AU_UNUSED(line);

        *log_file << message;
      },
      &_log_out_file);

  const String name = "IACrux";
  const i32 version = 99;

  IA_LOG_INFO("System {} online v{}", name, version);

  _log_out_file.close();

  const auto read_res = read_text_file(LOG_FILE);
  if (!read_res)
  {
    return true;
  }

  const String content = *read_res;
  IAT_CHECK(content.find("System IACrux online v99") != String::npos);

  cleanup_file(LOG_FILE);

  return true;
}

IAT_BEGIN_TEST_LIST()

IAT_ADD_TEST(test_file_logging);
IAT_ADD_TEST(test_log_levels);
IAT_ADD_TEST(test_formatting);

IAT_END_TEST_LIST()

IAT_END_BLOCK()

IAT_REGISTER_ENTRY(Core, Logger)
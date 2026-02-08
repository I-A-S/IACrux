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

#include <crux/platform.hpp>
#include <iatest/iatest.hpp>

using namespace ia;

IAT_BEGIN_BLOCK(Core, Platform)

auto test_os_name() -> bool
{
  const char *const os_name = platform::get_operating_system_name();
  IAT_CHECK(os_name != nullptr);

  const String os(os_name);
  IAT_CHECK(!os.empty());

#if IA_PLATFORM_WINDOWS
  IAT_CHECK_EQ(os, String("Windows"));
#elif IA_PLATFORM_LINUX
  IAT_CHECK_EQ(os, String("Linux"));
#elif IA_PLATFORM_APPLE
  IAT_CHECK_EQ(os, String("MacOS"));
#elif IA_PLATFORM_WASM
  IAT_CHECK_EQ(os, String("WebAssembly"));
#endif

  return true;
}

auto test_arch_name() -> bool
{
  const char *const arch_name = platform::get_architecture_name();
  IAT_CHECK(arch_name != nullptr);

  const String arch(arch_name);
  IAT_CHECK(!arch.empty());

#if IA_ARCH_X64
  IAT_CHECK_EQ(arch, String("x86_64"));
#elif IA_ARCH_ARM64
  IAT_CHECK_EQ(arch, String("ARM64"));
#elif IA_ARCH_WASM
  IAT_CHECK_EQ(arch, String("WASM"));
#endif

  return true;
}

auto test_capabilities() -> bool
{
  const auto check_result = platform::check_cpu();
  IAT_CHECK(check_result.has_value());

  const auto &caps = platform::get_capabilities();

  volatile const auto has_crc = caps.hardware_crc32;
  AU_UNUSED(has_crc);

  return true;
}

#if IA_ARCH_X64
auto test_cpuid() -> bool
{
  Mut<i32> regs[4] = {0};

  platform::cpuid(0, 0, regs);

  IAT_CHECK(regs[0] >= 0);

  Mut<char> vendor[13];
  std::memset(vendor, 0, 13);

  std::memcpy(vendor, &regs[1], 4);
  std::memcpy(vendor + 4, &regs[3], 4);
  std::memcpy(vendor + 8, &regs[2], 4);
  vendor[12] = '\0';

  const String vendor_str(vendor);
  IAT_CHECK(!vendor_str.empty());

  const bool is_known = (vendor_str == "GenuineIntel" || vendor_str == "AuthenticAMD" || vendor_str == "KVMKVMKVM" ||
                         vendor_str == "Microsoft Hv" || vendor_str == "VBoxVBoxVBox");

  if (!is_known)
    std::cout << "      [Info] Unknown CPU Vendor: " << vendor_str << "\n";

  return true;
}
#endif

IAT_BEGIN_TEST_LIST()
IAT_ADD_TEST(test_os_name);
IAT_ADD_TEST(test_arch_name);
IAT_ADD_TEST(test_capabilities);
#if IA_ARCH_X64
IAT_ADD_TEST(test_cpuid);
#endif
IAT_END_TEST_LIST()

IAT_END_BLOCK()

IAT_REGISTER_ENTRY(Core, Platform)
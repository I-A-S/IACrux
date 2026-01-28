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

#if IA_ARCH_X64
#  ifdef _MSC_VER
#    include <intrin.h>
#  else
#    include <cpuid.h>
#    include <immintrin.h>
#  endif
#elif IA_ARCH_ARM64
#  include <arm_acle.h>
#  if defined(__linux__) || defined(__ANDROID__)
#    include <asm/hwcap.h>
#    include <sys/auxv.h>
#  endif
#endif

namespace ia::platform
{
  auto get_capabilities_mut() -> MutRef<Capabilities>
  {
    static Mut<Capabilities> s_capabilities;
    return s_capabilities;
  }

  auto get_capabilities() -> Ref<Capabilities>
  {
    return get_capabilities_mut();
  }

#if defined(IA_ARCH_X64)
  auto cpuid(const i32 function, const i32 sub_function, Mut<i32> out[4]) -> void
  {
#  ifdef _MSC_VER
    __cpuidex(reinterpret_cast<i32 *>(out), static_cast<i32>(function), static_cast<i32>(sub_function));
#  else
    Mut<u32> a = 0;
    Mut<u32> b = 0;
    Mut<u32> c = 0;
    Mut<u32> d = 0;
    __cpuid_count(function, sub_function, a, b, c, d);
    out[0] = static_cast<i32>(a);
    out[1] = static_cast<i32>(b);
    out[2] = static_cast<i32>(c);
    out[3] = static_cast<i32>(d);
#  endif
  }
#endif

  auto check_cpu() -> Result<void>
  {
#if defined(IA_ARCH_X64)
    Mut<i32> cpu_info[4];

    cpuid(0, 0, cpu_info);
    if (cpu_info[0] < 7)
      return fail("failed to get cpuid.");

    cpuid(1, 0, cpu_info);
    const bool osxsave = (cpu_info[2] & (1 << 27)) != 0;
    const bool avx = (cpu_info[2] & (1 << 28)) != 0;
    const bool fma = (cpu_info[2] & (1 << 12)) != 0;
    get_capabilities_mut().hardware_crc32 = (cpu_info[2] & (1 << 20)) != 0;

    if (!osxsave || !avx || !fma)
      return fail("cpu is too old. this app needs a cpu that supports osxsave, avx1 and fma.");

    const u64 xcr_feature_mask = _xgetbv(0);
    if ((xcr_feature_mask & 0x6) != 0x6)
      return fail("cpu is too old. this app needs a cpu that supports xcr.");

    cpuid(7, 0, cpu_info);

    get_capabilities_mut().has_avx2 = (cpu_info[1] & (1 << 5)) != 0;

#elif defined(IA_ARCH_ARM64)
#  if defined(__linux__) || defined(__ANDROID__)
    const usize hw_caps = getauxval(AT_HWCAP);

#    ifndef HWCAP_CRC32
#      define HWCAP_CRC32 (1 << 7)
#    endif

    s_capabilities.hardware_crc32 = (hw_caps & HWCAP_CRC32) != 0;
#  elif defined(IA_PLATFORM_APPLE)
    s_capabilities.hardware_crc32 = true;
#  else
    s_capabilities.hardware_crc32 = false;
#  endif
#else
    s_capabilities.hardware_crc32 = false;
#endif
    return {};
  }

  auto get_architecture_name() -> const char *
  {
#if defined(IA_ARCH_X64)
    return "x86_64";
#elif defined(IA_ARCH_ARM64)
    return "aarch64";
#elif defined(IA_ARCH_WASM)
    return "wasm";
#else
    return "unknown";
#endif
  }

  auto get_operating_system_name() -> const char *
  {
#if IA_PLATFORM_WINDOWS
    return "Windows";
#elif defined(IA_PLATFORM_APPLE)
#  if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    return "iOS";
#  else
    return "macOS";
#  endif
#elif defined(__ANDROID__)
    return "Android";
#elif IA_PLATFORM_LINUX
    return "Linux";
#elif IA_PLATFORM_WASM
    return "WebAssembly";
#else
    return "Unknown";
#endif
  }
} // namespace ia::platform
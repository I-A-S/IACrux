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

namespace ia
{
  namespace platform
  {
    struct Capabilities
    {
      Mut<bool> has_avx2 = false;
      Mut<bool> hardware_crc32 = false;
    };

    auto check_cpu() -> Result<void>;

#if IA_ARCH_X64
    auto cpuid(const i32 function, const i32 sub_function, Mut<i32 *> out) -> void;
#endif

    auto get_architecture_name() -> const char *;
    auto get_operating_system_name() -> const char *;

    auto get_capabilities() -> Ref<Capabilities>;

  }; // namespace platform
} // namespace ia
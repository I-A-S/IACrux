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

#include <crux/crux.hpp>
#include <crux/platform.hpp>

#include <thread>

namespace ia::crux
{
  struct State
  {
    Mut<i32> init_counter{};
    Mut<std::thread::id> main_thread_id{};
  };

  auto get_state() -> MutRef<State>
  {
    static Mut<State> s_state{};
    return s_state;
  }

  auto initialize() -> void
  {
    auto &state = get_state();

    state.init_counter++;
    if (state.init_counter > 1)
      return;

    const auto cpu_is_supported = platform::check_cpu();
    if (!cpu_is_supported)
    {
      std::cerr << "unsupported cpu: " << cpu_is_supported.error() << ". aborting..\n";
      exit(-1);
    }

    state.main_thread_id = std::this_thread::get_id();
  }

  auto terminate() -> void
  {
    auto &state = get_state();

    state.init_counter--;
    if (state.init_counter > 0)
      return;
  }

  auto is_initialized() -> bool
  {
    return get_state().init_counter > 0;
  }

  auto is_main_thread() -> bool
  {
    return get_state().main_thread_id == std::this_thread::get_id();
  }
} // namespace ia::crux
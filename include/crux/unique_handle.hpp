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
  template<typename HandleT, HandleT InvalidValue, auto DestructorT> class UniqueHandle
  {
public:
    UniqueHandle() = default;

    UniqueHandle(const UniqueHandle &h) = delete;
    UniqueHandle &operator=(const UniqueHandle &) = delete;

    UniqueHandle(UniqueHandle &&h) : m_handle(std::exchange(h.m_handle, InvalidValue))
    {
    }

    UniqueHandle &operator=(UniqueHandle &&h)
    {
      if (this != &h)
      {
        reset();
        m_handle = std::exchange(h.m_handle, InvalidValue);
      }
      return *this;
    }

    ~UniqueHandle()
    {
      reset();
    }

    operator HandleT() const
    {
      return m_handle;
    }

    operator const HandleT &() const
    {
      return m_handle;
    }

    HandleT *ptr()
    {
      return &m_handle;
    }

    const HandleT *ptr() const
    {
      return &m_handle;
    }

    void reset()
    {
      if (m_handle != InvalidValue)
        DestructorT(m_handle);
      m_handle = InvalidValue;
    }

private:
    HandleT m_handle{InvalidValue};
  };

  template<typename HandleT, typename ManagerT, HandleT InvalidValue, auto DestructorT> class UniqueDependentHandle
  {
public:
    UniqueDependentHandle() = default;

    UniqueDependentHandle(ManagerT manager, HandleT &&handle) : m_manager(manager), m_handle(std::move(handle))
    {
    }

    UniqueDependentHandle(const UniqueDependentHandle &h) = delete;
    UniqueDependentHandle &operator=(const UniqueDependentHandle &) = delete;

    UniqueDependentHandle(UniqueDependentHandle &&h)
        : m_manager(h.m_manager), m_handle(std::exchange(h.m_handle, InvalidValue))
    {
    }

    UniqueDependentHandle &operator=(UniqueDependentHandle &&h)
    {
      if (this != &h)
      {
        reset();
        m_manager = h.m_manager;
        m_handle = std::exchange(h.m_handle, InvalidValue);
      }
      return *this;
    }

    ~UniqueDependentHandle()
    {
      reset();
    }

    operator HandleT() const
    {
      return m_handle;
    }

    operator const HandleT &() const
    {
      return m_handle;
    }

    HandleT *ptr()
    {
      return &m_handle;
    }

    const HandleT *ptr() const
    {
      return &m_handle;
    }

    void reset()
    {
      if ((m_manager != InvalidValue) && (m_handle != InvalidValue))
        DestructorT(m_manager, m_handle);
      m_handle = InvalidValue;
    }

private:
    ManagerT m_manager{};
    HandleT m_handle{InvalidValue};
  };
} // namespace ia

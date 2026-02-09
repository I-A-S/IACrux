# IACrux (Independent Architecture Crux)

<div align="center">
  <img src="logo.png" alt="IACrux Logo" width="400"/>
  <br/>
  
  <img src="https://img.shields.io/badge/license-apache_v2-blue.svg" alt="License"/>
  <img src="https://img.shields.io/badge/standard-C%2B%2B20-yellow.svg" alt="C++ Standard"/>
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg" alt="Platform"/>

  <p>
    <b>The centralized, high-performance bedrock of the IA ecosystem.</b>
  </p>
</div>

## **About**
IACrux is a foundation library designed to serve as the dependency root for Independent Architecture (IA) software.

Unlike general-purpose libraries, IACrux is opinionated and built specifically to streamline the development of our higher-level rendering, physics, and system tools. It provides a unified C++20 interface for OS abstractions, memory manipulation, and utilities, abstracting away platform-specific implementations so future projects can focus purely on logic.

Note: This project is tailored for our internal specific development environments and enforces the use of Clang across all platforms (Windows, Linux, and WebAssembly).

## **Core Modules**

### **1. Core & Environment (`crux.hpp`, `env.hpp`)**

The entry point for the library. 

It handles platform detection (`IA_PLATFORM_WINDOWS`, `IA_PLATFORM_LINUX`, `IA_ARCH_X64`, etc.) and global initialization.

* **Environment:** Cross-platform access to environment variables.
* **Global Types:** Aliases `ankerl::unordered_dense` to `ia::HashMap` and `ia::HashSet`.  

### **2. Shared Memory Ring Buffer (`adt/ring_buffer.hpp`)**

A specialized **Single-Producer Single-Consumer (SPSC)** lock-free ring buffer designed for **Inter-Process Communication (IPC)** via shared memory.

* **View Semantics:** RingBufferView does not allocate memory. It wraps a raw Span<u8> (e.g., a memory-mapped file), making it ideal for zero-copy IPC.  
* **Control Block:** The header of the buffer contains atomic read/write offsets, ensuring memory safety across process boundaries.  
* **Binary Packets:** Supports variable-length binary payloads with a PacketHeader.

### **3. Zero-Allocation Logging (`logger.hpp`)**

A callback-based logging system using std::source_location for automatic file/line tracking without macros in the call site (though macros are provided for convenience).

* **Callback Dispatch:** The logger does not output text itself; it formats the message and dispatches it to a user-defined Handler.  
* **Compile-time Stripping:** Trace and Debug logs are compiled out completely in non-debug builds.

### **4. Platform & Utils (`platform.hpp`, `utils.hpp`)**

* **Hardware Detection:** Runtime detection of *AVX2*, *CRC32 hardware* instructions, and CPU topology.  
* **Hashing:** `FNV1a`, `xxHash`, and `CRC32` implementations.  
* **Reflection-like Hashing:** The `IA_MAKE_HASHABLE` macro automatically generates `std::hash` (via `Ankerl`) specializations for custom structs.

## **Usage Examples**

### **1. Initialization**

You must initialize the library at the start of your application on the main thread.

```cpp
#include <crux/crux.hpp>

int main(int argc, char* argv[]) {
    AU_UNUSED(argc);
    AU_UNUSED(argv);
  
    ia::crux::initialize();

    if constexpr (ia::env::IS_WINDOWS) {  
        // ...  
    }

    ia::crux::terminate();  
    return 0;  
}
```

### **2. Advanced Logging**

Redirect logs to stdout or a file by setting a handler.

```cpp
#include <crux/logger.hpp>  
#include <crux/io.hpp>

void my_log_handler(void* user_data, ia::logger::Level level, ia::StringView msg, ia::StringView file, ia::u32 line) {  
    // Custom formatting  
    std::cout << "[" << file << ":" << line << "] " << msg << "n";  
}

void setup() {  
    ia::logger::set_handler(my_log_handler, nullptr);  
      
    // Uses std::format syntax  
    IA_LOG_INFO("System ready. CPU: {}", ia::platform::get_architecture_name());   
}
```

### **3. IPC Ring Buffer (Shared Memory)**

We have included a robust `RingBufferView` in IACrux, that can be used to build high-performance inter-communication systems such as IPC. 

```cpp
#include <crux/adt/ring_buffer.hpp>  

void ipc_example() {  
    // 1. Allocate raw memory (In real IPC, this comes from mmap/Shm)  
    Vec<ia::u8> raw_memory(1024);  
      
    // 2. Create the view as the Owner (initialize the ControlBlock)  
    auto result = ia::RingBufferView::create(raw_memory, true);  
      
    if (result) {  
        ia::RingBufferView& rb = result.value();  
          
        // 3. Push a packet  
        ia::String msg = "Hello IPC";  
        ia::u16 packet_id = 10;  
          
        // Push raw bytes  
        auto push_res = rb.push(packet_id, ia::Span<const ia::u8>((ia::u8*)msg.data(), msg.size()));  
          
        if (!push_res) {  
            IA_LOG_ERROR("Buffer full!");  
        }  
    }  
}
```

### **4. Automatic Struct Hashing**

Make any struct usable as a key in `ia::HashMap` instantly.

```cpp
#include <crux/utils.hpp>

struct Vector3 {  
    float x, y, z;  
};

// Injects specialization for ankerl::unordered_dense::hash  
IA_MAKE_HASHABLE(Vector3, &Vector3::x, &Vector3::y, &Vector3::z);

void usage() {  
    ia::HashMap<Vector3, String> point_names;  
    point_names[{1.0f, 0.0f, 0.0f}] = "Right";  
}
```

## **Integration**

### **CMake (FetchContent)**

```cmake
include(FetchContent)

FetchContent_Declare(  
    IACrux  
    GIT_REPOSITORY https://github.com/I-A-S/IACrux.git  
    GIT_TAG        main   
)  
FetchContent_MakeAvailable(IACrux)

add_executable(MyApp src/main.cpp)  
target_link_libraries(MyApp PRIVATE IACrux)
```

## **License**

Licensed under the Apache License, Version 2.0. See LICENSE for details.

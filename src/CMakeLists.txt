set(SRC_FILES
    "cpp/crux.cpp"
    "cpp/logger.cpp"
    "cpp/platform.cpp"
    "cpp/utils.cpp"
    "cpp/env.cpp"
)

add_library(IACrux STATIC ${SRC_FILES})

target_include_directories(IACrux PUBLIC ${IACRUX_ROOT}/include)
target_include_directories(IACrux PRIVATE hpp)

target_link_libraries(IACrux PUBLIC
    Auxid
    unordered_dense::unordered_dense
)

target_compile_definitions(IACrux PUBLIC 
    $<$<CONFIG:Debug>:__IA_DEBUG=1>
    $<$<CONFIG:Release>:__IA_DEBUG=0>
)

if(MSVC)
    set(NO_EXCEPT_FLAG "/EHs-c-")
else()
    set(NO_EXCEPT_FLAG "-fno-exceptions")
endif()
target_compile_options(IACrux PRIVATE ${NO_EXCEPT_FLAG})
target_compile_options(IACrux INTERFACE
    $<$<NOT:$<BOOL:$<TARGET_PROPERTY:USE_EXCEPTIONS>>>:${NO_EXCEPT_FLAG}>
)
define_property(TARGET PROPERTY USE_EXCEPTIONS
    BRIEF_DOCS "If ON, this target is allowed to use C++ exceptions."
    FULL_DOCS "Prevents IACrux from propagating -fno-exceptions to this target."
)

target_compile_definitions(IACrux PRIVATE 
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)


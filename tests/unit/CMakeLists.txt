set(SRC_FILES
  main.cpp

  env.cpp
  utils.cpp
  logger.cpp
  platform.cpp
)

add_executable(IACrux_Test_Suite ${SRC_FILES})

target_link_libraries(IACrux_Test_Suite PRIVATE
  IATest
  IACrux
)

set_target_properties(IACrux_Test_Suite PROPERTIES USE_EXCEPTIONS ON)

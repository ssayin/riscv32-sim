if(ENABLE_TCP)
  add_library(rv32-irq-server OBJECT src/irq_server.cpp)
  target_include_directories(rv32-irq-server PUBLIC include)

  add_executable(tcp_client examples/tcp_client.cpp)
  target_link_libraries(tcp_client rv32-irq-server)

  add_executable(minimal_test unittest/minimal_test.cpp)
  target_link_libraries(minimal_test rv32-irq-server)

  add_test(NAME minimal_test COMMAND minimal)
endif()

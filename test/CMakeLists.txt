add_executable(
    test
    expression_tests.c
    )

target_compile_options(test PRIVATE -g)

target_link_libraries(test
  PRIVATE
    skin_engine
  )

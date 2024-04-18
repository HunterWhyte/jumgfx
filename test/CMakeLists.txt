add_executable(
    units
    units.c
    )

target_compile_options(units PRIVATE -g)

target_link_libraries(units
  PRIVATE
    skin_engine
  )

add_executable(
  parse
  parse.c
  )

target_compile_options(parse PRIVATE -g)

target_link_libraries(parse
  PRIVATE
    skin_engine
  )

add_library(arithmetic INTERFACE)
target_sources(arithmetic
  INTERFACE
    ${INCLUDE_DIR}/global/arithmetic/arithmetic.hpp
  )

target_include_directories(arithmetic
  INTERFACE
    ${INCLUDE_DIR}/global/arithmetic/
  )

target_code_coverage(arithmetic INTERFACE)

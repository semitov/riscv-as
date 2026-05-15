cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(stv-assembler C CXX ASM)


include_directories(include)
include_directories(gperf)

set(HASH_OUTPUT_FILE "${PROJECT_SOURCE_DIR}/gperf/hash.c")
set_source_files_properties("${HASH_OUTPUT_FILE}" PROPERTIES GENERATED TRUE)

add_custom_target( hashMap
  COMMAND cd ${PROJECT_SOURCE_DIR}/gperf && gperf -o -t -I -C --output-file="${HASH_OUTPUT_FILE}" instructions.gperf
  COMMENT "Generating hash.c hash table file."
)

# Cut out arguments parsing and other stuff
add_compile_definitions(PICO_BUILD)

add_library(stv-assembler
    src/debug.c
    src/instruction.c
    src/writer.c
    "${HASH_OUTPUT_FILE}"
)
add_dependencies(stv-assembler hashMap)
target_include_directories(stv-assembler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.16)
project(muTauPlatformIO C CXX)

# Path passed in: base/build/tang_nano_9k
if(NOT DEFINED LITEX_BUILD_DIR)
    message(FATAL_ERROR "LITEX_BUILD_DIR must be set (path to base/build/<board>).")
endif()

# LiteX-generated headers and makefile are in software/include/generated
set(LITEX_GEN_DIR "${LITEX_BUILD_DIR}/software/include/generated")
set(LITEX_INC_DIR "${LITEX_BUILD_DIR}/software/include")

set(LITEX_ROOT_DIR "${CMAKE_SOURCE_DIR}/base/litex")
set(SOC_INC_DIR    "${LITEX_ROOT_DIR}/litex/soc/software/include")
set(CPU_INC_DIR    "${LITEX_ROOT_DIR}/litex/soc/cores/cpu/vexriscv")

set(LITEX_VARS_MAK "${LITEX_GEN_DIR}/variables.mak")
if(NOT EXISTS "${LITEX_VARS_MAK}")
    message(FATAL_ERROR "LiteX variables.mak not found at ${LITEX_VARS_MAK}")
endif()

file(READ "${LITEX_VARS_MAK}" VARS_MAK_CONTENT)

function(extract_make_var VAR_NAME OUT_NAME)
    string(REGEX MATCH "${VAR_NAME}[ \t]*:=?[ \t]*([^\n]*)"
           _match "${VARS_MAK_CONTENT}")
    if(_match)
        string(REGEX REPLACE "${VAR_NAME}[ \t]*:=?[ \t]*" "" _value "${_match}")
        string(STRIP "${_value}" _value)
        set(${OUT_NAME} "${_value}" PARENT_SCOPE)
    else()
        set(${OUT_NAME} "" PARENT_SCOPE)
    endif()
endfunction()

extract_make_var("CFLAGS"  LITEX_CFLAGS)
extract_make_var("LDFLAGS" LITEX_LDFLAGS)
extract_make_var("LIBS"    LITEX_LIBS)

message(STATUS "LiteX CFLAGS:  ${LITEX_CFLAGS}")
message(STATUS "LiteX LDFLAGS: ${LITEX_LDFLAGS}")
message(STATUS "LiteX LIBS:    ${LITEX_LIBS}")

# Apply LiteX flags
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${LITEX_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LITEX_CFLAGS}")

# Use LiteX LDFLAGS (adds regions.ld/output_format.ld from LITEX_GEN_DIR)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LITEX_LDFLAGS}")

# Add the search path for the generated linker scripts
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${LITEX_GEN_DIR}")

# Add our SRAM-resident linker script on top
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/linker.ld")

add_executable(${PROJECT_NAME}
    src/main.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${LITEX_INC_DIR}"
    "${LITEX_GEN_DIR}"
    "${SOC_INC_DIR}"
    "${CPU_INC_DIR}"
)

# Link against LiteX-provided libs from variables.mak (may be empty)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LITEX_LIBS})

# Output ELF and BIN
set(BIN_OUTPUT "${CMAKE_BINARY_DIR}/barebone.bin")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "barebone")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND /opt/riscv-toolchain/bin/riscv64-unknown-elf-objcopy -O binary
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${BIN_OUTPUT}"
    COMMENT "Converting ELF to BIN: ${BIN_OUTPUT}"
)

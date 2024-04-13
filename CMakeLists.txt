# Imported from https://github.com/fkutzner/lingeling-cmake, commit afb0f5

cmake_minimum_required(VERSION 3.19)
project(lingeling)

option(LGL_BUILD_TOOLS "Build the Lingeling executables in addition to the library" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

set(LGL_GENERATED_SRC_DIR "${CMAKE_BINARY_DIR}/generated_src")



# Set up installation for Lingeling, so it can be added to other projects
# using CMAKE_PREFIX_PATH and find_package(Lingeling):
install(EXPORT Lingeling DESTINATION lib/cmake/Lingeling FILE "LingelingConfig.cmake")



write_file("${LGL_GENERATED_SRC_DIR}/lglcfg.h" [=[
  #define LGL_OS "<os info omitted>"
  #define LGL_COMPILED "<compile date omitted>"
  #define LGL_RELEASED "<release date omitted>"
  #define LGL_VERSION "1.0.0"
  #define LGL_ID "<git commit ID omitted>"
]=])

write_file("${LGL_GENERATED_SRC_DIR}/lglcflags.h" [=[
  #define LGL_CC "<C compiler name omitted>"
  #define LGL_CFLAGS "<C flags omitted>"
]=])



add_compile_options(
  # Disable all lingeling extras for now:
  -DNLGLOG
  -DNCHKSOL
  -DNLGLDRUPLIG
  -DNLGLYALSAT
  -DNLGLFILES
  -DNLGLDEMA

  # lingeling uses uint64_t, but does not include stdint.h, so force it:
  -include stdint.h

  # Deliberately omitting warning flags (-Wall -Wextra -pedantic), because
  # this fork avoids touching original source files
)



# The targets `lingeling-static` and `lingeling-shared` are the main Lingeling libraries.
# Their names on disk match `liblgl.*` corresponding to the original build script, which
# produces a static library `liblgl.a`.
# TODO: deal with ELF pessimization: https://maskray.me/blog/2021-05-16-elf-interposition-and-bsymbolic
set(LGL_LIB_SOURCES lglbnr.c lgldimacs.c lglib.c lglopts.c ipasir2.c)
add_library(lingeling-shared SHARED ${LGL_LIB_SOURCES})
add_library(lingeling-static STATIC ${LGL_LIB_SOURCES})

foreach (target IN ITEMS lingeling-shared lingeling-static)
  target_link_libraries(${target} PUBLIC -lm)
  target_include_directories(${target} PRIVATE "${LGL_GENERATED_SRC_DIR}")
  target_include_directories(${target} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME "lgl")
endforeach()

install(FILES "${PROJECT_SOURCE_DIR}/lglib.h" DESTINATION include)
install(TARGETS lingeling-shared lingeling-static EXPORT Lingeling LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


if (LGL_BUILD_TOOLS)
  function(add_tool name sourcefile)
    add_executable(${name} ${sourcefile})
    target_link_libraries(${name} PRIVATE lingeling-static)
    set_target_properties(${name} PROPERTIES OUTPUT_NAME ${name})
    install(TARGETS ${name} EXPORT Lingeling RUNTIME DESTINATION bin)
  endfunction()

  add_tool(lingeling lglmain.c)
  add_tool(plingeling plingeling.c)
  add_tool(treengeling treengeling.c)
  add_tool(lglmbt lglmbt.c)
  add_tool(lgluntrace lgluntrace.c)
  add_tool(lglddtrace lglddtrace.c)

  # blimc requires AIGER, so it is excludede for now:
  # add_tool(blimc blimc.c)
endif()

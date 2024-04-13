# Imported from https://github.com/fkutzner/lingeling-cmake, commit 5c10eaf9

cmake_minimum_required(VERSION 3.19)
project(lingeling)

option(LGL_BUILD_TOOLS "Build the Lingeling executables in addition to the library" ON)
option(LGL_BUILD_FAKE_POSIX "Experimental: don't assume a POSIX environment, but provide enough functions to compile the lingeling library. Disables LGL_BUILD_TOOLS." OFF)

if (LGL_BUILD_FAKE_POSIX)
  set(LGL_BUILD_TOOLS OFF)
endif()

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
set(LGL_LIB_SOURCES lglbnr.c lgldimacs.c lglib.c lglopts.c)
add_library(lingeling-shared SHARED ${LGL_LIB_SOURCES})
add_library(lingeling-static STATIC ${LGL_LIB_SOURCES})

foreach (target IN ITEMS lingeling-shared lingeling-static)
  target_link_libraries(${target} PUBLIC -lm)
  target_include_directories(${target} PRIVATE "${LGL_GENERATED_SRC_DIR}")
  target_include_directories(${target} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME "lgl" PREFIX "lib")
endforeach()

install(FILES "${PROJECT_SOURCE_DIR}/lglib.h" DESTINATION include)
install(TARGETS lingeling-shared lingeling-static EXPORT Lingeling LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

if (WIN32)
  # lglib.h does not have symbol visibility information, so export all symbols:
  set_target_properties(lingeling-shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()



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



if (LGL_BUILD_FAKE_POSIX)
  # This code is currently included here to keep the repo's overall
  # diff to the original as small as possible.

  write_file("${LGL_GENERATED_SRC_DIR}/sys/resource.h" [=[
    // This header fakes POSIX functions used by lingeling, see CMakeLists.txt

    #include <time.h>

    struct lgl_timeval {
      time_t tv_sec;
      long int tv_usec;
    };

    struct lgl_rusage {
      struct lgl_timeval ru_utime;
      struct lgl_timeval ru_stime;
    };

    #define RUSAGE_SELF 0

    #define rusage lgl_rusage

    // Exporting all symbols, so don't clobber getrusage:
    #define getrusage lgl_getrusage

    int lgl_getrusage(int who, struct lgl_rusage *r_usage);
  ]=])

  write_file("${LGL_GENERATED_SRC_DIR}/sys/resource.c" [=[
    #include <sys/resource.h>
    #include <stdint.h>

    int lgl_getrusage(int who, struct lgl_rusage *r_usage) {
      clock_t cpu_time = clock();
      if (cpu_time < 0) {
        return -1;
      }

      int64_t cpu_usecs = cpu_time * 1000000 / CLOCKS_PER_SEC;
      r_usage->ru_utime.tv_sec = cpu_usecs / 1000000;
      r_usage->ru_utime.tv_usec = cpu_usecs % 1000000;
      r_usage->ru_stime.tv_sec = 0;
      r_usage->ru_stime.tv_usec = 0;
      return 0;
    }
  ]=])

  write_file("${LGL_GENERATED_SRC_DIR}/sys/time.h" [=[
    // Stub
  ]=])

  target_sources(lingeling-shared PRIVATE "${LGL_GENERATED_SRC_DIR}/sys/resource.c")
  target_sources(lingeling-static PRIVATE "${LGL_GENERATED_SRC_DIR}/sys/resource.c")
endif()

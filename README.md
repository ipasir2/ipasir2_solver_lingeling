# Lingeling with IPASIR-2 API

This is an experimental fork of the [Lingeling](https://github.com/arminbiere/lingeling)
1.0.0 SAT solver, with the CMake build of [lingeling-cmake](https://github.com/fkutzner/lingeling-cmake).

The primary purpose of this fork is to ensure that IPASIR-2 can be implemented
in a pure C implementation without too much trouble.

See also the [original README file](README). This version of Lingeling is published
under the MIT license, see [COPYING](COPYING).


## Building `ipasir2_solver_lingeling`

`ipasir2_solver_lingeling` builds Lingeling both as a static and as a shared library. The
build artifacts are placed in the `target` subdirectory of your build.

The Lingeling executables are built using the static Lingeling library. If you only
need the Lingeling libraries, set the CMake option `LGL_BUILD_TOOLS` to `OFF`.

When building with MinGW32, pass `-DLGL_BUILD_FAKE_POSIX=ON` to CMake. This option
currently provides a very simple implementation of the POSIX `getrusage()` function,
as far as it is used by the Lingeling library. This option disables building the
Lingeling executables, since they use further POSIX functions which cannot be faked
easily. To help static linking into clients that might define `getrusage()`
otherwise (other libraries might do similar tricks, for example), a custom function
symbol is used for the fake implementation.


## Integrating `ipasir2_solver_lingeling`

If the `ipasir2_solver_lingeling` sources are included in your source tree (for example as a
git submodule), you can simply create the targets by adding it via `add_subdirectory()`.

Otherwise, a `ipasir2_solver_lingeling` installation can be used as a CMake package in the
usual fashion:
- When setting up the `ipasir2_solver_lingeling` build, pass an installation directory to
  CMake via `CMAKE_INSTALL_PREFIX`
- Build the `install` target
- Add the installation directory to your client's `CMAKE_PREFIX_PATH`
- In your client CMake code, add `find_package(Lingeling)`

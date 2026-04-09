# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-src")
  file(MAKE_DIRECTORY "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-src")
endif()
file(MAKE_DIRECTORY
  "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-build"
  "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix"
  "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix/tmp"
  "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix/src/lua51-populate-stamp"
  "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix/src"
  "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix/src/lua51-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix/src/lua51-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Spel/RoRebuild/Ragnarok___Win32_HighPriest2008_Release/build-mingw-preset-debug/_deps/lua51-subbuild/lua51-populate-prefix/src/lua51-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

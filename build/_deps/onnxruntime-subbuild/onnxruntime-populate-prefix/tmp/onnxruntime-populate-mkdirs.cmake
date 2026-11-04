# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-src")
  file(MAKE_DIRECTORY "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-src")
endif()
file(MAKE_DIRECTORY
  "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-build"
  "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix"
  "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix/tmp"
  "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix/src/onnxruntime-populate-stamp"
  "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix/src"
  "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix/src/onnxruntime-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix/src/onnxruntime-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/_deps/onnxruntime-subbuild/onnxruntime-populate-prefix/src/onnxruntime-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

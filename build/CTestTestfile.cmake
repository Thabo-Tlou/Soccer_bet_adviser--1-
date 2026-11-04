# CMake generated Testfile for 
# Source directory: /home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/Soccer_bet_adviser
# Build directory: /home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[core]=] "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/sba_tests")
set_tests_properties([=[core]=] PROPERTIES  WORKING_DIRECTORY "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/Soccer_bet_adviser" _BACKTRACE_TRIPLES "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/Soccer_bet_adviser/CMakeLists.txt;120;add_test;/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/Soccer_bet_adviser/CMakeLists.txt;0;")
add_test([=[api]=] "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/build/sba_api_tests")
set_tests_properties([=[api]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/Soccer_bet_adviser/CMakeLists.txt;141;add_test;/home/osboxes/Documents/GitHub/Soccer_bet_adviser--1-/Soccer_bet_adviser/CMakeLists.txt;0;")
subdirs("_deps/nlohmann_json-build")

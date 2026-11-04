# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/SoccerBetAdviser_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/SoccerBetAdviser_autogen.dir/ParseCache.txt"
  "CMakeFiles/sba_api_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/sba_api_tests_autogen.dir/ParseCache.txt"
  "SoccerBetAdviser_autogen"
  "sba_api_tests_autogen"
  )
endif()

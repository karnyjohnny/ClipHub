# CMake generated Testfile for 
# Source directory: /app/applet
# Build directory: /app/applet/build-win64
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[CoreTests]=] "/app/applet/build-win64/cliphub_tests.exe")
set_tests_properties([=[CoreTests]=] PROPERTIES  _BACKTRACE_TRIPLES "/app/applet/CMakeLists.txt;77;add_test;/app/applet/CMakeLists.txt;0;")

# CMake generated Testfile for 
# Source directory: /workspaces/obcpp
# Build directory: /workspaces/obcpp
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_all "./bin/hello_test2")
set_tests_properties(test_all PROPERTIES  _BACKTRACE_TRIPLES "/workspaces/obcpp/CMakeLists.txt;62;add_test;/workspaces/obcpp/CMakeLists.txt;0;")
subdirs("deps/google-test")

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

# https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
FetchContent_Declare(Torch
  URL https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.1.0%2Bcpu.zip
)
  # URL https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip

FetchContent_MakeAvailable(Torch)
# find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${TORCH_LIBRARIES}"
)
include_directories(SYSTEM ${TORCH_INCLUDE_DIRS})

# find_package(Torch REQUIRED)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# add_executable(example-app example-app.cpp)
# target_link_libraries(example-app "${TORCH_LIBRARIES}")
# set_property(TARGET example-app PROPERTY CXX_STANDARD 17)



# add_executable(example-app example-app.cpp)
# target_link_libraries(example-app "${TORCH_LIBRARIES}")
# set_property(TARGET example-app PROPERTY CXX_STANDARD 17)
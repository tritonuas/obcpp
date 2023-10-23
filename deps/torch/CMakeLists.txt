include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

# https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
FetchContent_Declare(Torch
  # URL https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip
  URL https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.1.0%2Bcpu.zip
)

FetchContent_MakeAvailable(Torch)
# find_package(Torch REQUIRED)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${TORCH_LIBRARIES}"
)



# add_executable(example-app example-app.cpp)
# target_link_libraries(example-app "${TORCH_LIBRARIES}")
# set_property(TARGET example-app PROPERTY CXX_STANDARD 17)
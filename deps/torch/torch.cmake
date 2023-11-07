function(target_add_torch target_name)
    message("Adding torch as a dependency for target ${target_name}")

    include(FetchContent)
    Set(FETCHCONTENT_QUIET FALSE)

    FetchContent_Declare(Torch
      URL https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.1.0%2Bcpu.zip
    )

    FetchContent_MakeAvailable(Torch)
    find_package(Torch REQUIRED HINTS "${torch_SOURCE_DIR}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
    target_link_libraries(${target_name} PRIVATE
        "${TORCH_LIBRARIES}"
    )
    target_include_directories(${target_name} PRIVATE ${TORCH_INCLUDE_DIRS})
endfunction()
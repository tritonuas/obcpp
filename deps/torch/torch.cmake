function(target_add_torch target_name)
    message("Adding torch as a dependency for target ${target_name}")

    # specify a CMAKE_PREFIX_PATH that points to torch installation
    find_package(Torch REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
    target_link_libraries(${target_name} PRIVATE ${TORCH_LIBRARIES})
    target_include_directories(${target_name} PRIVATE ${TORCH_INCLUDE_DIRS})
endfunction()
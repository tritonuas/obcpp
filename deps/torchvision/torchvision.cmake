function(target_add_torchvision target_name)
    message("Adding torchvision as a dependency for target ${target_name}")

    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)

    FetchContent_Declare(TorchVision
        URL https://github.com/pytorch/vision/archive/refs/tags/v0.16.0.zip
    )

    # FetchContent_Declare(TorchVision
    #     GIT_REPOSITORY https://github.com/pytorch/vision
    #     GIT_PROGRESS TRUE
    #     GIT_SHALLOW TRUE
    #     GIT_TAG v0.16.0
    # )

    FetchContent_MakeAvailable(TorchVision)

    # message(FATAL_ERROR "torchvision source dir: ${TorchVision_SOURCE_DIR}")

    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/_deps/torchvision-src")
    set(TorchVision_DIR "${CMAKE_BINARY_DIR}/_deps/torchvision-src")

    find_package(TorchVision REQUIRED)
    target_link_libraries(${target_name} PRIVATE TorchVision::TorchVision)
endfunction()
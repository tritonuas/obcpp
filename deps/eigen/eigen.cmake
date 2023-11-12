function(target_add_eigen target_name)
    set(EIGEN_DIR "${CMAKE_BINARY_DIR}/_deps/eigen-src")

    include(FetchContent)

    FetchContent_Declare(eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_TAG 3.4.0)

    FetchContent_MakeAvailable(eigen)

    target_include_directories(${target_name} PRIVATE 
        "${EIGEN_DIR}/Eigen"
    )
endfunction()
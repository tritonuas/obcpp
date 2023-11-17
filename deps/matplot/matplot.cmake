function(target_add_matplot target_name)
    include(FetchContent)

    # TODO: Why is this download so slow?
    # The binaries on the release page are 4MB, we should use those.
    FetchContent_Declare(matplotplusplus
            URL https://github.com/alandefreitas/matplotplusplus/archive/refs/tags/v1.2.0.zip
            URL_HASH SHA256=282446b30a285ea593ee7baaf6fb8978551fd3671e78b1b869270a62d1decf3b
            DOWNLOAD_EXTRACT_TIMESTAMP true
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
    )
            
    FetchContent_GetProperties(matplotplusplus)
    if(NOT matplotplusplus_POPULATED)
        FetchContent_Populate(matplotplusplus)
        add_subdirectory(${matplotplusplus_SOURCE_DIR} ${matplotplusplus_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    target_link_libraries(${target_name} PRIVATE
        matplot
    )
endfunction()
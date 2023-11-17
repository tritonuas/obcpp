function(target_add_matplot target_name)
    include(FetchContent)

    # TODO: Why is this download so slow?
    # The binaries on the release page are 4MB, we should use those.
    FetchContent_Declare(matplotplusplus
            GIT_REPOSITORY https://github.com/alandefreitas/matplotplusplus
            GIT_TAG origin/master # v1.2.0
            # URL https://github.com/alandefreitas/matplotplusplus/archive/refs/tags/v1.2.0.zip
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
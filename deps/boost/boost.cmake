function(target_add_boost target_name)
    include(FetchContent)

    FetchContent_Declare(boost 
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-b2-nodocs.tar.gz
        URL_HASH SHA256=d6c69e4459eb5d6ec208250291221e7ff4a2affde9af6e49c9303b89c687461f
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(boost)
    # target_link_libraries(${target_name} PRIVATE
    #     boost
    # )
    target_include_directories(${target_name} PRIVATE ${boost_SOURCE_DIR})


endfunction()

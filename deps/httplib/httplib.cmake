function(target_add_httplib target_name)
    include(FetchContent)

    FetchContent_Declare(httplib 
        URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.14.1.tar.gz
        URL_HASH SHA256=2d4fb5544da643e5d0a82585555d8b7502b4137eb321a4abbb075e21d2f00e96
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(httplib)
    target_link_libraries(${target_name} PRIVATE
        httplib
    )

endfunction()
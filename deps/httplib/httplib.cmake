function(target_add_httplib target_name)
    include(FetchContent)

    FetchContent_Declare(httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.14.1.tar.gz)
    FetchContent_MakeAvailable(httplib)
    target_link_libraries(${target_name} PRIVATE
        httplib
    )

endfunction()
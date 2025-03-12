function(target_add_onnxruntime target_name)
    include(FetchContent)
    
    FetchContent_Declare(onnxruntime 
        URL https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-aarch64-1.20.1.tgz
        URL_HASH SHA256=ae4fedbdc8c18d688c01306b4b50c63de3445cdf2dbd720e01a2fa3810b8106a
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(onnxruntime)

    # After FetchContent_MakeAvailable(onnxruntime)
    if(EXISTS "${onnxruntime_SOURCE_DIR}/include/onnxruntime_cxx_api.h")
    execute_process(
        COMMAND sed -i "210s|^|//|" "${onnxruntime_SOURCE_DIR}/include/onnxruntime_cxx_api.h"
    )
    execute_process(
        COMMAND sed -i "351s|^|//|" "${onnxruntime_SOURCE_DIR}/include/onnxruntime_cxx_api.h"
    )
    endif()

    
    # Add the include directory for the headers.
    target_include_directories(${target_name} PUBLIC 
        ${onnxruntime_SOURCE_DIR}/include
    )
    
    # Tell the linker where to find the onnxruntime library.
    target_link_directories(${target_name} PUBLIC 
        ${onnxruntime_SOURCE_DIR}/lib
    )
    
    # Now link against onnxruntime.
    target_link_libraries(${target_name} PUBLIC onnxruntime)
endfunction()

function(target_add_onnxruntime target_name)
    include(FetchContent)

    # Detect architecture.
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-1.20.1.tgz")
        set(ONNX_HASH SHA256=67db4dc1561f1e3fd42e619575c82c601ef89849afc7ea85a003abbac1a1a105)
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-aarch64-1.20.1.tgz")
        set(ONNX_HASH SHA256=ae4fedbdc8c18d688c01306b4b50c63de3445cdf2dbd720e01a2fa3810b8106a)
    else()
        message(FATAL_ERROR "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    FetchContent_Declare(
        onnxruntime
        URL      ${ONNX_URL}
        URL_HASH ${ONNX_HASH}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(onnxruntime)

    # Apply patches
    if(EXISTS "${onnxruntime_SOURCE_DIR}/include/onnxruntime_cxx_api.h")
        execute_process(
            COMMAND sed -i "210s|^|//|" "${onnxruntime_SOURCE_DIR}/include/onnxruntime_cxx_api.h"
        )
        execute_process(
            COMMAND sed -i "351s|^|//|" "${onnxruntime_SOURCE_DIR}/include/onnxruntime_cxx_api.h"
        )
    endif()

    target_include_directories(${target_name} PUBLIC
        ${onnxruntime_SOURCE_DIR}/include
    )
    target_link_directories(${target_name} PUBLIC
        ${onnxruntime_SOURCE_DIR}/lib
    )
    target_link_libraries(${target_name} PUBLIC onnxruntime)
endfunction()

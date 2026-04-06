# Option to enable GPU/TensorRT support
option(ONNXRUNTIME_USE_GPU "Enable ONNX Runtime GPU (CUDA/TensorRT) support" OFF)

function(target_add_onnxruntime target_name)
    include(FetchContent)

    # Detect platform and architecture
    if(WIN32)
        # Windows (x86_64 / AMD64) — TensorRT not supported on Windows
        if(ONNXRUNTIME_USE_GPU)
            message(WARNING "ONNXRUNTIME_USE_GPU is not supported on Windows (TensorRT is Linux-only). Falling back to CPU.")
        endif()
        set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-win-x64-1.20.1.zip")
        set(ONNX_HASH SHA256=8e2b1e2d47ad09b5b1f7491b48d5ad0f47cad54c6aa0cf33da21e5688613c4b9)
        message(STATUS "Using ONNX Runtime CPU build for Windows x64")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if(ONNXRUNTIME_USE_GPU)
            # GPU build with CUDA and TensorRT support
            set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-gpu-1.20.1.tgz")
            set(ONNX_HASH SHA256=a117e3e389a0e6c34432fa1c6ead067d2d4e79a87b8ef4d5d19d5e88a6c85a93)
            message(STATUS "Using ONNX Runtime GPU build for x86_64 (CUDA/TensorRT enabled)")
        else()
            set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-1.20.1.tgz")
            set(ONNX_HASH SHA256=67db4dc1561f1e3fd42e619575c82c601ef89849afc7ea85a003abbac1a1a105)
            message(STATUS "Using ONNX Runtime CPU build for x86_64")
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        if(ONNXRUNTIME_USE_GPU)
            # For Jetson: use the JetPack-compatible GPU build
            # Note: Requires TensorRT and CUDA from JetPack to be installed on the system
            set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-aarch64-1.20.1.tgz")
            set(ONNX_HASH SHA256=ae4fedbdc8c18d688c01306b4b50c63de3445cdf2dbd720e01a2fa3810b8106a)
            message(STATUS "Using ONNX Runtime for aarch64 (TensorRT via system libs)")
        else()
            set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-aarch64-1.20.1.tgz")
            set(ONNX_HASH SHA256=ae4fedbdc8c18d688c01306b4b50c63de3445cdf2dbd720e01a2fa3810b8106a)
            message(STATUS "Using ONNX Runtime CPU build for aarch64")
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform/architecture: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    FetchContent_Declare(
        onnxruntime
        URL      ${ONNX_URL}
        URL_HASH ${ONNX_HASH}
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE
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

    # Define compile flag if GPU support is enabled
    if(ONNXRUNTIME_USE_GPU)
        target_compile_definitions(${target_name} PUBLIC ONNXRUNTIME_USE_GPU=1)
    endif()
endfunction()

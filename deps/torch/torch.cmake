function(target_add_torch target_name)
  message("Adding torch as a dependency for target ${target_name}")

  include(FetchContent)
  Set(FETCHCONTENT_QUIET FALSE)

  FetchContent_Declare(Torch
    URL https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.1.0%2Bcpu.zip
  )

  FetchContent_MakeAvailable(Torch)
  find_package(Torch REQUIRED HINTS "${torch_SOURCE_DIR}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
  target_link_libraries(${target_name} PRIVATE
    "${TORCH_LIBRARIES}"
  )
  target_include_directories(${target_name} PRIVATE ${TORCH_INCLUDE_DIRS})

  # Install torchvision
  set(INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/deps/torch/install-torchvision.sh")

  set(TORCHVISION_INSTALL_LOCATION "${CMAKE_BINARY_DIR}/_deps/torchvision")
  # Get version number from commit tags from this repo: https://github.com/pytorch/vision/releases
  # Make sure to remove the "v" prefix from the tag. Use "0.16.0" and NOT "v0.16.0". Otherwise, horrible things will happen.
  set(TORCHVISION_VERSION "0.16.0")

  execute_process(
    COMMAND sh ${INSTALL_SCRIPT} 
    ${TORCHVISION_INSTALL_LOCATION} 
    ${TORCHVISION_VERSION} 
    ${torch_SOURCE_DIR}
    RESULT_VARIABLE ret
  )

  if(NOT ret EQUAL "0")
    message(FATAL_ERROR "Unable to install torchvision. ${INSTALL_SCRIPT} script exited with code ${ret}.")
  endif()

  set(TorchVision_DIR "${TORCHVISION_INSTALL_LOCATION}/vision-${TORCHVISION_VERSION}/build")
  set(CMAKE_CURRENT_LIST_DIR "${TORCHVISION_INSTALL_LOCATION}/vision-${TORCHVISION_VERSION}/build")
  find_package(TorchVision REQUIRED HINTS "${TORCHVISION_INSTALL_LOCATION}/vision-${TORCHVISION_VERSION}/build")
  # target_link_libraries(${target_name} PRIVATE TorchVision::TorchVision)

endfunction()
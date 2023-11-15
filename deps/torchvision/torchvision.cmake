function(target_add_torchvision target_name)
    # Get version number from commit tags from this repo: https://github.com/pytorch/vision/releases
    set(TORCHVISION_VERSION "v0.16.0")
    set(TORCHVISION_INSTALL_LOCATION "${CMAKE_BINARY_DIR}/_deps/torchvision")


endfunction()
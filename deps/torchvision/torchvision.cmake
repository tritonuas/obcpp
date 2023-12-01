function(target_add_torchvision target_name)
    message("Adding torchvision as a dependency for target ${target_name}")
    # Expects torchvision to be installed to the system.
    # The devcontainer for this project does so already.

    # https://github.com/pytorch/vision#using-the-models-on-c
    find_package(TorchVision REQUIRED)
    # Make sure to include --no-as-needed to ensure that torchvision gets linked with the target.
    # Otherwise, torchvision will not be linked together since we never call any torchvision code.
    # Torchvision functions are called by the torchscript JIT when it finds some torchvision operator 
    # that's not included default torch.
    # See how it's setup on this Facebook detectron repo: 
    # https://github.com/facebookresearch/detectron2/blob/main/tools/deploy/CMakeLists.txt#L21
    target_link_libraries(${target_name} PRIVATE -Wl,--no-as-needed TorchVision::TorchVision)
endfunction()

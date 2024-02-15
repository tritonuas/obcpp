function(target_add_imagemagick target_name)
    add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
    add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )

    find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
    target_include_directories(${target_name} PRIVATE ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})
endfunction()
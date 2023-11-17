function (target_add_mavsdk target_name)
  # mavsdk is on the PATH as it is installed to the container
  find_package(MAVSDK REQUIRED)

  target_link_libraries(${target_name} PRIVATE 
    MAVSDK::mavsdk # All apps link against mavsdk library
  )
endfunction()
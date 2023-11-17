function (target_add_mavsdk target_name)
  find_package(MAVSDK REQUIRED)

  target_link_libraries(${target_name} PRIVATE 
    MAVSDK::mavsdk             #All apps link against mavsdk library
  )
endfunction()
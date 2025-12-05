function(target_add_opencv target_name)
  # Use system OpenCV installation from PATH
  find_package(OpenCV REQUIRED)

  # Mark OpenCV headers as SYSTEM to suppress warnings from external library
  include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${target_name} PRIVATE ${OpenCV_LIBS})
endfunction()
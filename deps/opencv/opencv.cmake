function(target_add_opencv target_name)
  find_package(OpenCV REQUIRED)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${target_name} PRIVATE ${OpenCV_LIBS})

  # The following is for building opencv from source (not currently working)
  # include(FetchContent)

  # # Fetch the main OpenCV repository
  # FetchContent_Declare(OpenCV
  #   URL https://github.com/opencv/opencv/archive/refs/tags/4.8.1.tar.gz
  #   OVERRIDE_FIND_PACKAGE
  # )

  # # set( OPENCV_EXTRA_MODULES_PATH ${OpenCV_SOURCE_DIR}/../opencvcontrib-src/modules)
  # # Fetch the OpenCV contrib repository
  # FetchContent_Declare(OpenCVContrib
  #   URL https://github.com/opencv/opencv_contrib/archive/refs/tags/4.8.1.tar.gz
  #   # SOURCE_DIR "${CMAKE_BINARY_DIR}/opencv_contrib-src" # Specify a separate source directory
  #   # PREFIX "${CMAKE_BINARY_DIR}/opencv_contrib-prefix"  # Specify a separate prefix directory
  # )

  # # Make both OpenCV and OpenCV contrib available
  # FetchContent_MakeAvailable(OpenCV OpenCVContrib)

  # # Find OpenCV
  # find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui features2d flann dnn imgcodecs videoio imgproc ml)
  # set(OpenCV_INCLUDE_DIRS ${OpenCV_SOURCE_DIR}/include )
  # # file(GLOB_RECURSE HEADERS "${OpenCV_SOURCE_DIR}/modules/**/*.hpp")
  # # Add OpenCV's contrib modules to the target
  # target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} opencv_core opencv_highgui opencv_features2d opencv_flann opencv_imgcodecs opencv_dnn opencv_videoio opencv_imgproc opencv_ml )

  # # Specify include directories, including OpenCV's contrib
  # target_include_directories(${PROJECT_NAME} PRIVATE
  #     ${OpenCV_INCLUDE_DIRS}
  #     ${OpenCVContrib_SOURCE_DIR}/modules  # Include contrib modules
  #     ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
  #     ${OPENCV_MODULE_opencv_core_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_highgui_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_features2d_LOCATION}/include

  #     ${OPENCV_MODULE_opencv_flann_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_dnn_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_videoio_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_imgproc_LOCATION}/include
  #     ${OPENCV_MODULE_opencv_ml_LOCATION}/include
  #     # ${HEADERS}
  # )
endfunction()
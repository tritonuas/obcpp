include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)
#  cmake .. -DBUILD_TESTS=OFF
# add_compile_options(-Wno-deprecated-enum-enum-conversion)
# https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
FetchContent_Declare(OpenCV
  URL               https://github.com/opencv/opencv/archive/refs/tags/4.8.1.tar.gz
  OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(OpenCV)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
# Set(OpenCV_DIR "/home/samir/Documents/github/tuas/obcpp/deps/opencv/opencv-build")
# add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/opencv-src
#                  ${CMAKE_CURRENT_BINARY_DIR}/opencv-build)
# file(GLOB_RECURSE HEADERS "./**/*.hpp")
# include_directories(${PROJECT_NAME} ${OpenCV_INCLUDE_DIRS} ${HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/opencv-src "./opencv-src/include/opencv2/opencv.hpp")
# target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} )#${HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/opencv-src "./opencv-src/include/opencv2/opencv.hpp")
# target_link_libraries(${PROJECT_NAME} PRIVATE
#     ${OpenCV_LIBS}
# )

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-enum-enum-conversion)

file(GLOB_RECURSE HEADERS "include/*.hpp")

message("OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")
set(OpenCV_INCLUDE_DIRS ${OpenCV_SOURCE_DIR}/include )
message("OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS}         ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
        ${OPENCV_MODULE_opencv_core_LOCATION}/include
        ${OPENCV_MODULE_opencv_highgui_LOCATION}/include ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} opencv_core opencv_highgui)

# Set(  "OpenCV_DIR" "opencv-build" )
# set(OpenCV_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

# https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
FetchContent_Declare(OpenCV
  URL               https://github.com/opencv/opencv/archive/refs/tags/4.8.1.tar.gz
  )

FetchContent_MakeAvailable(OpenCV)

include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBS}
)

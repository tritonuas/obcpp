function(target_add_json target_name)
  include(FetchContent)

  # https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v3.11.2)

  FetchContent_MakeAvailable(json)

  target_link_libraries(${target_name} PRIVATE
      nlohmann_json::nlohmann_json
  )
endfunction()
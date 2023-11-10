find_package(MAVSDK REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    MAVSDK::mavsdk             #All apps link against mavsdk library
)
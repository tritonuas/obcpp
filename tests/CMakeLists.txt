# so everything in unit and integration tests can include header files in this directory
include_directories(util)

add_compile_definitions(CONFIG_DIRECTORY="${PROJECT_SOURCE_DIR}/configs")

add_subdirectory(integration)

add_subdirectory(unit)
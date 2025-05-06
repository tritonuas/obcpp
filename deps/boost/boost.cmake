function(target_add_boost target_name)
	include(FetchContent)

    FetchContent_Declare(boost 
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-b2-nodocs.tar.gz
        URL_HASH SHA256=d6c69e4459eb5d6ec208250291221e7ff4a2affde9af6e49c9303b89c687461f
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(boost)
    # target_link_libraries(${target_name} PRIVATE
    #     boost
    # )
    target_include_directories(${target_name} PRIVATE ${boost_SOURCE_DIR})

endfunction()

# --- Just for Serialization ): ---
include(FetchContent)
message(STATUS "Setting up Boost 1.87.0 via FetchContent...")

FetchContent_Declare(
    Boost_Source 
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.87.0 
)

set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Force static Boost libraries" FORCE)

# OPTION A (Example Variable - Verify Name):
# set(BOOST_LIBS_TO_BUILD serialization CACHE STRING "Specific Boost libraries to build" FORCE)
# OPTION B (Alternative Example - Verify Name):
# set(BOOST_INCLUDE_LIBRARIES serialization CACHE STRING "Specific Boost libraries to build" FORCE)

message(WARNING "Boost build configuration needs the correct variable set to specify building 'serialization' for Boost 1.87.0 CMake superproject!")


message(STATUS "Boost_Source declared. Configuring and making available...")
FetchContent_MakeAvailable(Boost_Source)

if(TARGET Boost::serialization)
    get_target_property(BoostSerializationType Boost::serialization TYPE)
    if(BoostSerializationType STREQUAL "INTERFACE_LIBRARY")
        # This means CMake created a target but didn't link it to a compiled library
        message(FATAL_ERROR "Boost::serialization target exists but is INTERFACE only. Boost build DID NOT compile serialization sources. Check Boost build configuration variables above (BOOST_LIBS_TO_BUILD or similar for 1.87.0)!")
    else()
        message(STATUS "Boost::serialization target type: ${BoostSerializationType}. Looks like a compiled library target exists.")
    endif()
     message(STATUS "Boost FetchContent setup complete.")
else()
     message(FATAL_ERROR "Target Boost::serialization was NOT created by FetchContent_MakeAvailable(Boost_Source). Check Boost build configuration variables and logs in build/_deps/boost_source-subbuild/boost_source-populate-*.log")
endif()

function(target_add_loguru target_name)
    include(FetchContent)
    FetchContent_Declare(LoguruGitRepo
        URL "https://github.com/emilk/loguru/archive/4adaa185883e3c04da25913579c451d3c32cfac1.zip"
        URL_HASH SHA256=da272cae7a177217a02fbb2622bd0ce80f3b0b6a4ffb71579c186d80ecf81f2e
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )

    # This line is necessary to enable logging with streams in loguru
    # (i.e. LOG_S(mode) << "Log message"; )
    # You need to include this line, AND #define LOGURU_WITH_STREAMS 1
    # to enable logging with streams
    set(LOGURU_WITH_STREAMS TRUE)

    FetchContent_MakeAvailable(LoguruGitRepo) # defines target 'loguru::loguru'

    target_link_libraries(${target_name} PRIVATE
        loguru::loguru 
    )
endfunction()
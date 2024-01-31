function(target_add_loguru target_name)
    include(FetchContent)
    FetchContent_Declare(LoguruGitRepo
        GIT_REPOSITORY "https://github.com/emilk/loguru" # can be a filesystem path
        GIT_TAG        "master"
    )

    # set any loguru compile-time flags before calling MakeAvailable()
    set(LOGURU_WITH_STREAMS TRUE)
    FetchContent_MakeAvailable(LoguruGitRepo) # defines target 'loguru::loguru'

    target_link_libraries(${target_name} PRIVATE
        loguru::loguru 
    )
endfunction()
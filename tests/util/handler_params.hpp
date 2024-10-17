#ifndef TESTS_HELPERS_HPP_
#define TESTS_HELPERS_HPP_

#define DECLARE_HANDLER_PARAMS(STATE, REQ, RESP) \
    int argc = 5; \
    char argv0[] = "bin/obcpp"; \
    char argv1[] = CONFIG_DIRECTORY; \
    char argv2[] = "dev"; \
    char argv3[] = "stickbug"; \
    char argv4[] = "sitl"; \
    char *argv[] = {argv0, argv1, argv2, argv3, argv4, nullptr }; \
    std::shared_ptr<MissionState> STATE = std::make_shared<MissionState>(OBCConfig(argc, argv)); \
    httplib::Request REQ; \
    httplib::Response RESP 

#endif  // TESTS_HELPERS_HPP_
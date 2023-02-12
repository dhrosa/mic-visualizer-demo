diy_cc_library(buffer AUTO LIBRARIES absl::any_invocable)
diy_cc_test(buffer_test AUTO)

add_library(test_main test_main.cc)
target_link_libraries(test_main PRIVATE gmock gtest
                                        absl::failure_signal_handler)

diy_cc_library(rational AUTO)

diy_cc_test(rational_test AUTO)

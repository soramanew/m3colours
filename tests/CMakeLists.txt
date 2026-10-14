find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(m3colours_tests
    quantise/celebi_test.cpp
    quantise/wsmeans_test.cpp
    quantise/wu_test.cpp
)
target_link_libraries(m3colours_tests PRIVATE m3colours::m3colours GTest::gtest_main)
gtest_discover_tests(m3colours_tests)

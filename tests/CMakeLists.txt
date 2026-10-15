find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(m3colours_tests
    blend/blend_test.cpp
    contrast/contrast_test.cpp
    dislike/dislike_test.cpp
    hct/hct_round_trip_test.cpp
    hct/hct_test.cpp
    quantise/celebi_test.cpp
    quantise/wsmeans_test.cpp
    quantise/wu_test.cpp
)
target_link_libraries(m3colours_tests PRIVATE m3colours::m3colours GTest::gtest_main)
gtest_discover_tests(m3colours_tests)

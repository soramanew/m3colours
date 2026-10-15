find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(m3colours_tests
    blend/blend_test.cpp
    contrast/contrast_test.cpp
    dislike/dislike_test.cpp
    dynamiccolour/colour_spec_test.cpp
    dynamiccolour/dynamic_colour_test.cpp
    dynamiccolour/dynamic_scheme_test.cpp
    dynamiccolour/fixed_dynamic_colour_test.cpp
    hct/hct_round_trip_test.cpp
    hct/hct_test.cpp
    palettes/palettes_test.cpp
    quantise/celebi_test.cpp
    quantise/wsmeans_test.cpp
    quantise/wu_test.cpp
    scheme/scheme_test.cpp
    score/score_test.cpp
    temperature/temperature_test.cpp
)
target_link_libraries(m3colours_tests PRIVATE m3colours::m3colours GTest::gtest_main)
gtest_discover_tests(m3colours_tests)

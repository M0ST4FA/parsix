file(GLOB EXAMPLE_SRCs "./*.cpp")
add_executable(parsixExamples ${EXAMPLE_SRCs})
target_link_libraries(parsixExamples PRIVATE parsix gtest)

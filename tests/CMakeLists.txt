# VARIABLES ------------------------------

# TESTS ----------------------------------

# LL Parser Tests
add_executable(LLParserTests "ll parser.cpp" "Tests.ixx" "universal.cpp" "ll parser.h" "universal.h")

target_sources(LLParserTests 
PRIVATE
FILE_SET modules TYPE CXX_MODULES 
FILES "ParserTests.cpp"
)

target_include_directories(LLParserTests PRIVATE
"${GTEST_INCLUDE_DIR}/../"
)
target_link_libraries(LLParserTests PRIVATE parsix gtest_main)
gtest_discover_tests(LLParserTests
TEST_PREFIX ParserTests
)
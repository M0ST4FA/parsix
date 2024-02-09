# CMakeList.txt : CMake project for myacc, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.27)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(parsix)

# SETUP VARIABLES

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API aa1f7df0-828a-4fcd-9afc-2dc80491aca7)

set(${PROJECT_NAME}_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

file(GLOB SRC_FILES     "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB INCLUDE_FILES "${${PROJECT_NAME}_INCLUDE_DIR}/parsix/*.h")

# ADD THE LIBRARY
add_library(${PROJECT_NAME} STATIC 
${SRC_FILES}
${INCLUDE_FILES}
)

add_subdirectory("${PROJECT_SOURCE_DIR}/external/lexana/")

target_link_libraries(${PROJECT_NAME} PUBLIC lexana)
target_include_directories(${PROJECT_NAME} PUBLIC 
	$<BUILD_INTERFACE:${${PROJECT_NAME}_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:include>
)

# ADD THE EXAMPLES
if(${BUILD_EXAMPLES})
	add_subdirectory("./examples/")
endif()

# ADD THE TESTS
if(${BUILD_TESTING})
	enable_testing()
	# add_subdirectory("./tests/")
endif()

# BUILD THE DOCUMENTATION
if(${BUILD_DOCUMENTATION})
	add_subdirectory("./docs/")
endif()

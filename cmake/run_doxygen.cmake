# Require dot, treat the other components as optional
find_package(Doxygen
			REQUIRED dot
			OPTIONAL_COMPONENTS mscgen dia)

if (${DOXYGEN_FOUND})
	# We begin by declaring variables that will be used by the `doxygen_add_docs` command to define properties of the same name in the configuration file.

	# This directory is relative to the binary directory
	set( DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/docs/" )
	set( DOXYGEN_COLLABORATION_GRAPH YES )
	set( DOXYGEN_EXTRACT_ALL YES )
	set( DOXYGEN_CLASS_DIAGRAMS YES )
	set( DOXYGEN_HIDE_UNDOC_RELATIONS NO )
	set( DOXYGEN_HAVE_DOT YES )
	set( DOXYGEN_CLASS_GRAPH YES )
	set( DOXYGEN_CALL_GRAPH YES )
	set( DOXYGEN_CALLER_GRAPH YES )
	set( DOXYGEN_COLLABORATION_GRAPH YES )
	set( DOXYGEN_BUILTIN_STL_SUPPORT YES )
	set( DOXYGEN_EXTRACT_PRIVATE YES )
	set( DOXYGEN_EXTRACT_PACKAGE YES )
	set( DOXYGEN_EXTRACT_STATIC YES )
	set( DOXYGEN_EXTRACT_LOCALMETHODS YES )
	set( DOXYGEN_UML_LOOK YES )
	set( DOXYGEN_UML_LIMIT_NUM_FIELDS 50 )
	set( DOXYGEN_TEMPLATE_RELATIONS YES )
	set( DOXYGEN_DOT_GRAPH_MAX_NODES 100 )
	set( DOXYGEN_MAX_DOT_GRAPH_DEPTH 0 )
	set( DOXYGEN_DOT_TRANSPARENT YES )
	set( DOXYGEN_GENERATE_XML YES)
	set( DOXYGEN_EXCLUDE_PATTERNS 
	"*/docs/*"
	"*/tests/*"
	"*/out/*"
	"*/examples/*"
	"*/external/*"
	)

	# This creates a target to run Doxygen on this project. The name of the target is the first argument.
	# In addition, this also sets the extension of the configuration file. The second argument is the root of the search for doxygen.
	doxygen_add_docs(${PROJECT_NAME}.docs ${CMAKE_SOURCE_DIR})

	set(DOXYGEN_CONFIGURATION_FILE "${DOXYGEN_OUTPUT_DIRECTORY}/Doxyfile.${PROJECT_NAME}.docs")

	configure_file(
		"${PROJECT_SOURCE_DIR}/docs/Sphinx/source/conf.py.in"
		"${PROJECT_SOURCE_DIR}/docs/Sphinx/source/conf.py"
	)

	add_custom_target(Doxygen_${PROJECT_NAME} ALL
	COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIGURATION_FILE}
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen"
	)

endif()
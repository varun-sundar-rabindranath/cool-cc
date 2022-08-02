

## Make Parsing Files

# Execute the parsing_files_generator_app from the COOL-CC project to generate
# the parsing files for the arith grammar
set(PARSING_FILES_GENERATOR_CMD parsing_files_generator_app)
set(GRAMMAR_DEFINITION_FILENAME ${CMAKE_CURRENT_SOURCE_DIR}/arith.grammar)
set(PARSING_FILES_GENERATOR_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
execute_process(COMMAND ${PARSING_FILES_GENERATOR_CMD}
                --grammar-definition-filename ${GRAMMAR_DEFINITION_FILENAME}
                --output-directory ${PARSING_FILES_GENERATOR_OUTPUT_DIRECTORY}
                OUTPUT_VARIABLE CMD_OUT
                ERROR_VARIABLE CMD_ERR
		RESULT_VARIABLE CMD_RET)

message(STATUS "  - Input : " ${GRAMMAR_DEFINITION_FILENAME} " , " ${PARSING_FILES_GENERATOR_OUTPUT_DIRECTORY})
message(STATUS "  - Output :\n" ${CMD_OUT})

message(STATUS ${PARSING_FILES_GENERATOR_CMD})

if(NOT ${CMD_RET} EQUAL 0)
  message(STATUS "  - Error :\n " ${CMD_ERR})
  message(FATAL_ERROR ${PARSING_FILES_GENERATOR_CMD} " failed")
endif()

## Build arith library

set(LIBRARY_NAME arith_grammar)

add_library(${LIBRARY_NAME}
	    SHARED
	    parsing_table.cpp
	    semantic_rules.cpp)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(FILES arith_ast.hpp DESTINATION include)

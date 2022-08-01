

## Make Parsing Files

# Execute the parsing_files_generator_app from the COOL-CC project to generate
# the parsing files for the arith grammar
set(PARSING_FILES_GENERATOR_CMD parsing_files_generator_app)
set(GRAMMAR_DEFINITION_FILENAME arith.grammar)
set(PARSING_FILES_GENERATOR_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
execute_process(COMMAND ${PARSING_FILES_GENERATOR_CMD}
                --grammar-definition-filename ${GRAMMAR_DEFINITION_FILENAME}
                --output-directory ${PARSING_FILES_GENERATOR_OUTPUT_DIRECTORY}
                OUTPUT_VARIABLE CMD_OUT
                ERROR_VARIABLE CMD_ERR)

message(STATUS ${PARSING_FILES_GENERATOR_CMD} " - " ${GRAMMAR_DEFINITION_FILENAME}
               "Output - " ${CMD_OUT}
               "Error - " ${CMD_ERR})

## Build arith library

set(LIBRARY_NAME arith_grammar)

add_library(${LIBRARY_NAME}
	    SHARED
	    parsing_table.cpp
	    semantic_rules.cpp)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(FILES arith_ast.hpp DESTINATION include)

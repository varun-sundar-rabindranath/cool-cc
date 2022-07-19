

## dfa_test ####
set(DFA_TEST_EXECUTABLE_NAME dfa_test)

add_executable(${DFA_TEST_EXECUTABLE_NAME}
	       dfa_test.cpp)

target_link_libraries(${DFA_TEST_EXECUTABLE_NAME}
		      lexer
		      errorhandler
		      utils
		      fmt::fmt)

install(TARGETS ${DFA_TEST_EXECUTABLE_NAME} DESTINATION bin)

## lexer_test ####
set(LEXER_TEST_EXECUTABLE_NAME lexer_test)

add_executable(${LEXER_TEST_EXECUTABLE_NAME}
	       lexer_test.cpp)

target_link_libraries(${LEXER_TEST_EXECUTABLE_NAME}
		      lexer
		      errorhandler
		      utils
		      fmt::fmt)

install(TARGETS ${LEXER_TEST_EXECUTABLE_NAME} DESTINATION bin)

## parser_test ####
set(PARSER_TEST_EXECUTABLE_NAME parser_test)

add_executable(${PARSER_TEST_EXECUTABLE_NAME}
	       parser_test.cpp)

target_link_libraries(${PARSER_TEST_EXECUTABLE_NAME}
		      lexer
		      errorhandler
		      parser
		      utils
		      fmt::fmt)

install(TARGETS ${PARSER_TEST_EXECUTABLE_NAME} DESTINATION bin)

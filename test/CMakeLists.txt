

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

## recursive descent parser generator test ####
set(RDPG_TEST_EXECUTABLE_NAME recursive_descent_parser_generator_test)

add_executable(${RDPG_TEST_EXECUTABLE_NAME}
               recursive_descent_parser_generator_test.cpp)

target_link_libraries(${RDPG_TEST_EXECUTABLE_NAME}
		      errorhandler
                      parser_generator
		      utils
		      fmt::fmt)

install(TARGETS ${RDPG_TEST_EXECUTABLE_NAME} DESTINATION bin)

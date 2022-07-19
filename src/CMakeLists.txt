
set(COOLCC_EXECUTABLE_NAME cool_cc)

add_executable(${COOLCC_EXECUTABLE_NAME}
	       cool_cc.cpp)

target_link_libraries(${COOLCC_EXECUTABLE_NAME}
		      lexer
		      parser
		      errorhandler
		      utils
		      fmt::fmt)

install(TARGETS ${COOLCC_EXECUTABLE_NAME} DESTINATION bin)

add_subdirectory(utils)
add_subdirectory(error_handler)
add_subdirectory(lexer)
add_subdirectory(parser)

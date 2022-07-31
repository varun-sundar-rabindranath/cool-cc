# Parser Generator
set(LIBRARY_NAME parser_generator)

add_library(${LIBRARY_NAME}
            SHARED
            grammar_file_parser.cpp
            parser_generator.cpp
            recursive_descent_parser_generator.cpp)

target_link_libraries(${LIBRARY_NAME}
		      utils
		      fmt::fmt)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

# Recursive Descent Parser
set (LIBRARY_NAME recursive_descent_parser)

add_library(${LIBRARY_NAME}
            SHARED
            recursive_descent_parser.cpp)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

# Parsing Files Generator App
set(EXECUTABLE_NAME parsing_files_generator_app)

add_executable(${EXECUTABLE_NAME}
               parsing_files_generator_app.cpp)

target_link_libraries(${EXECUTABLE_NAME}
		      parser_generator
		      fmt::fmt)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

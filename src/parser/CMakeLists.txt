#set(LIBRARY_NAME parser)
#
#add_library(${LIBRARY_NAME}
#	    SHARED
#	    grammar_file_parser.cpp
#	    parser.cpp)
#
#target_link_libraries(${LIBRARY_NAME}
#		      utils
#		      fmt::fmt)
#
#install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

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

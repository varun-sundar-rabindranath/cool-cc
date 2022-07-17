set(LIBRARY_NAME parser)

add_library(${LIBRARY_NAME}
	    SHARED
	    grammar_file_parser.cpp
	    parser.cpp)

target_link_libraries(${LIBRARY_NAME}
		      utils
		      fmt::fmt)

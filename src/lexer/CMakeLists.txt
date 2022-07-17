set(LIBRARY_NAME lexer)

add_library(${LIBRARY_NAME}
	    SHARED
	    lexer.cpp
	    dfa.cpp
	    lex_character_classes.cpp
	    regex_tree_nodes.cpp)

target_link_libraries(${LIBRARY_NAME}
		      utils
		      errorhandler
		      fmt::fmt)

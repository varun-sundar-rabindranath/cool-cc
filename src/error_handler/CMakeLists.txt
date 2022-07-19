
set(LIBRARY_NAME errorhandler)

add_library(${LIBRARY_NAME}
	    SHARED
	    error_handler.cpp)

target_link_libraries(${LIBRARY_NAME}
		      fmt::fmt)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

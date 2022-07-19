
set(LIBRARY_NAME utils)

add_library(${LIBRARY_NAME}
	    SHARED
	    file_location.cpp
	    file_utils.cpp
	    string_utils.cpp)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

# Makefile is replaced by cmake - This is deprecated - Remove this file
BUILD_DIR=./build/
SOURCE_DIR=./src/
TEST_DIR=./test/
INCLUDE_DIRS = -I/home/varun/study/compilers/cool-cc/include
LIBRARIES= -L/home/varun/study/compilers/cool-cc/${BUILD_DIR}
LD_FLAGS= -l fmt
#CPP_FLAGS= -g -std=c++14 ${INCLUDE_DIRS} ${LIBRARIES} -DCCDEBUG
CPP_FLAGS= -g -std=c++14 ${INCLUDE_DIRS} ${LIBRARIES}
CPP= g++

# Define all sources
LEXER_DIR= ${SOURCE_DIR}/lexer
PARSER_DIR = ${SOURCE_DIR}/parser
UTILS_DIR= ${SOURCE_DIR}/utils
ERR_DIR= ${SOURCE_DIR}/error_handler
LEXER_SOURCES= ${LEXER_DIR}/lexer.cpp  	         \
	       ${LEXER_DIR}/dfa.cpp   	         \
	       ${LEXER_DIR}/regex_tree_nodes.cpp \
	       ${LEXER_DIR}/lex_character_classes.cpp
PARSER_SOURCES= ${PARSER_DIR}/parser.cpp 	\
		${PARSER_DIR}/grammar_file_parser.cpp
UTILS_SOURCES= ${UTILS_DIR}/string_utils.cpp 	\
	       ${UTILS_DIR}/file_utils.cpp 	\
	       ${UTILS_DIR}/file_location.cpp
ERR_SOURCES = ${ERR_DIR}/error_handler.cpp

# Define all objects
LEXER_OBJECTS=$(LEXER_SOURCES:.cpp=.o)
PARSER_OBJECTS=$(PARSER_SOURCES:.cpp=.o)
UTILS_OBJECTS=$(UTILS_SOURCES:.cpp=.o)
ERR_OBJECTS=$(ERR_SOURCES:.cpp=.o)

all: build_dir cool-cc

build_dir:
	mkdir -p ${BUILD_DIR}

cool-cc: utils err lexer parser
	${CPP} ${CPP_FLAGS} -o ${BUILD_DIR}/cool_cc ./src/cool_cc.cpp -l errhandler -l lexer -l utils -l parser ${LD_FLAGS}

lexer: utils err $(LEXER_OBJECTS)
	${CPP} ${CPP_FLAGS} -shared -o  ${BUILD_DIR}/liblexer.so ${LEXER_OBJECTS} -l utils -l errhandler ${LD_FLAGS}

parser: utils $(PARSER_OBJECTS)
	${CPP} ${CPP_FLAGS} -shared -o  ${BUILD_DIR}/libparser.so ${PARSER_OBJECTS} -l utils ${LD_FLAGS}

utils: $(UTILS_OBJECTS)
	${CPP} ${CPP_FLAGS} -shared -o ${BUILD_DIR}/libutils.so ${UTILS_OBJECTS} ${LD_FLAGS}

err: $(ERR_OBJECTS)
	${CPP} ${CPP_FLAGS} -shared -o ${BUILD_DIR}/liberrhandler.so ${ERR_OBJECTS} ${LD_FLAGS} -l utils

test: dfa_test
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/dfa_test

dfa_test: ${TEST_DIR}/dfa_test.cpp
	${CPP} ${CPP_FLAGS} -o ${BUILD_DIR}/dfa_test ./test/dfa_test.cpp -l errhandler -l lexer -l utils ${LD_FLAGS} 
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/dfa_test

lexer_test: ${TEST_DIR}/lexer_test.cpp
	${CPP} ${CPP_FLAGS} -o ${BUILD_DIR}/lexer_test ./test/lexer_test.cpp -l errhandler -l lexer -l utils ${LD_FLAGS} 
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/lexer_test --lexer-definition-filename ./data/lexer_regex.lex

parser_test: parser ${TEST_DIR}/parser_test.cpp
	${CPP} ${CPP_FLAGS} -o ${BUILD_DIR}/parser_test ./test/parser_test.cpp -l errhandler -l lexer -l parser -l utils ${LD_FLAGS} 

run_parser_test: parser_test
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/parser_test --test-compute-first --test-compute-follow --test-compute-parsing-table

run_parser_compute_first_test: parser_test
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/parser_test --test-compute-first

run_parser_compute_follow_test: parser_test
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/parser_test --test-compute-follow

run_parser_compute_parsing_table_test: parser_test
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/parser_test --test-compute-parsing-table

%.o: %.cpp
	${CPP} ${CPP_FLAGS} -fPIC -c -o $@ $^ ${LD_FLAGS}

purge:
	rm -rf ${BUILD_DIR}

rlexer:
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/cool_cc --lexer-definition-filename /home/varun/study/compilers/cool-cc/data/lexer_regex.lex --lexer -f $(coolprogram)

rparser:
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/ ./build/cool_cc --grammar-definition-filename /home/varun/study/compilers/cool-cc/data/arith/arith.grammar --parser


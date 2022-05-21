README
------
  - A COOL programming language compiler implementation attempt.
  - Lexer [Works]
  - All other compiler phases are TODO

Parser Design
-------------
  - How to define a Grammar:
    - Grammar file contents:
      - Must contain the productions
      - Must contain the Terminals
      - Must contain the Non terminals
      - Must contain the semantic rules
	- All semantic rules must return a ASTNode
    - AST definitions in a separate .h file [The appropriate .h file would be
      identified in the Makefile]
      - Every terminal and non-terminal must have AST Node definitions

  - ParserGenerator
    - The ParserGenerator should output 2 .h files
      - 1. All terminals, non-terminals, productions and the parsing table
      - 2. ASTNode definitions with information on which semantic rule to
	   call during parsing

  - Parser:
    - The parser will just read the two .h files and perform boiler-plate actions

Dependencies
------------
 - CLI11
 - spdlog
 - fmt::format

Project
-------
  - The compiler folder has all the compiler related passes that are programming
    language agnostic, like
    - lexer
    - parser
    - ...

  - The pl folder has all the programming language specific stuff - typically the
    definitions of the programming language (grammer, lex file, semantic rules etc.)

  - The compiler and pl folders are treated as separate projects and have their own
    CMakeLists.txt.
    - The compiler folder / project is stand-alone (doesn't depend on any other
      sub-projects)
    - The pl project build requires that the compiler project is already built

How to build
------------

  - make build

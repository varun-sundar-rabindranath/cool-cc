// Define the regex for all the tokens
// Each line is a key-value pair in the format key:value. The key part is the token
// the value part (inside the braces) defines the regex.
// Character classes
//   [1-9] - Digits 1 - 9
//   [0-9] - Digits 0 - 9
//   [a-z] - Lower Case a - z
//   [A-Z] - Upper Case A - Z
//   [abc] - letters a, b, c
//   [E-L] - End of Line, [\n]
//   [E-F] - End of File, [EOF]
//   [W-S] - White Space
// TODO: Nested comments
DEFINITIONS:
// Keywords
CLASS : {(class|Class)}
ELSE : {else}
FALSE : {false}
FI : {fi}
IF : {if}
IN : {in}
INHERITS : {inherits}
ISVOID : {isvoid}
LET : {let}
LOOP : {loop}
POOL : {pool}
THEN : {then}
WHILE : {while}
CASE : {case}
ESAC : {esac}
NEW : {new}
OF : {of}
NOT : {not}
TRUE : {true}
// Operators
PLUS : {+}
MINUS : {-}
TIMES : {\*}
DIVIDE : {/}
LT : {<}
LE : {<=}
EQUALS : {=}
LARROW : {<\-}
RARROW : {=>}
LBRACE : {\{}
RBRACE : {\}}
LPAREN : {\(}
RPAREN : {\)}
COLON : {:}
SEMI : {;}
DOT : {\.}
COMMA : {,}
TILDE : {~}
AT : {@}
INTEGER : {(0|[1-9]([0-9]*))}
IDENTIFIER : {[a-z]([A-Za-z0-9_]*)}
TYPE : {[A-Z]([A-Za-z_0-9]*)}
SELF_IDENTIFIER : {self}
SELF_TYPE : {SELF_TYPE}
STRING : {"((\\.|[W-S]|[^\\"])*)"}
COMMENT_LINE : {--(([^E-LE-F])*)([E-L]|[E-F])}
//// Handle the rest of the logic in the next part of lexer ????
COMMENT_BLOCK_START : {\(\*}
COMMENT_BLOCK_END : {\*\)}
WS : {[W-S]}
KEYWORDS:
CLASS
ELSE
FALSE
FI
IF
IN
INHERITS
ISVOID
LET
LOOP
POOL
THEN
WHILE
CASE
ESAC
NEW
OF
NOT
TRUE
SYMBOLS:
PLUS
MINUS
TIMES
DIVIDE
LT
LE
EQUALS
LARROW
RARROW
LBRACE
RBRACE
LPAREN
RPAREN
COLON
SEMI
DOT
COMMA
TILDE
AT

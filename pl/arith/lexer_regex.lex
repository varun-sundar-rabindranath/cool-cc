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
DEFINITIONS:
// Operators
+ : {+}
* : {\*}
( : {(}
) : {)}
id : {(0|[1-9]([0-9]*))}
KEYWORDS:

number   : /-?[0-9]+/ ;
dbl      : /-?[0-9]+[.][0-9]+/ ;
bool     : "true" | "false" ;
operator : '+' | '-' | '*' | '/' | "&&" | "||" | "==" | "<=" | ">=" | "!=" | '>' | '<' ;
ident    : /[a-zA-Z_][a-zA-Z0-9_]*/ ;
arg      : <func> | <expr> | <ident> ;
noarg    : "" ;
args     : (<arg> ',')* <arg> | <noarg> ;
func     : <ident> '(' <args> ')' ;
normal   : <str> | <func> | <dbl> | <number> | <bool> | <ident> | <gcomp> | <list> ;
comp     : <normal> <operator> <expr>;
gcomp    : '(' <comp> ')' ;
expr     : <str> | <comp> | <gcomp> | <normal> | <list> | '(' <operator> <expr>+ ')' ;
assign   : <ident> '=' <expr> ';' ;
var      : "var" <assign> ";" | "var" <ident> ";" ;
list     : '[' <args> ']' ;
str      : /\"(\\\\.|[^\"])*\"/ ;
state    : <var> | <assign> | <func> ';' | <cond> | <while> ;
if       : "if" <gcomp> <body> ;
else     : <if> "else" <body> | <elif> "else" <body> ;
felif    : <if> "elif" <gcomp> <body> ;
elif     : <felif> "elif" <gcomp> <body> | <felif> ;
while    : "while" <gcomp> <body> ;
cond     : <else> | <elif> | <if> ;
body     : '{' <state>* '}' ;
fundef   : "def" <func> <body> ;
toplevel : <fundef> | <state> | <comp> | <list> | <expr> ;
lispy    : /^/ <toplevel>+ /$/ ;

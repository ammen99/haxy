number   : /-?[0-9]+/ ;
dbl      : /-?[0-9]+[.][0-9]+/ ;
bool     : "true" | "false" ;
operator : '+' | '-' | "**" | '*' | '/' | "&&" | "||" | "==" | "<=" | ">=" | "!=" | '>' | '<' ;
ident    : /[a-zA-Z_][a-zA-Z0-9_]*/ ;
arg      : <func> | <expr> | <ident> ;
noarg    : "" ;
args     : (<arg> ',')* <arg> | <noarg> ;
func     : <ident> '(' <args> ')' ;
normal   : <listq> | <str> | <func> | <dbl> | <number> | <bool> | <ident> | <gcomp> | <list> ;
comp     : <normal> <operator> <expr>;
gcomp    : '(' <comp> ')' ;
expr     : <comp> | <gcomp> | <normal> | <list> | '(' <operator> <expr>+ ')' ;
assign   : <ident> '=' <expr> ;
var      : "var" (<assign> | <ident>) (',' (<assign> | <ident>))*  ;
modlist  : <listq> '=' <expr> ;
list     : '[' <args> ']' ;
str      : /\"(\\\\.|[^\"])*\"/ ;
state    : <modlist> ';' | <listq> | <return> | <var> ';' | <assign> ';' | <func> ';' | <cond> | <while> ;
if       : "if" <gcomp> <body> ;
elif     : "elif" <gcomp> <body> ;
else     : "else" <body> ;
cond     : <if> <elif>* <else>* ;
while    : "while" <gcomp> <body> ;
body     : '{' <state>* '}' ;
fundef   : "def" <func> <body> ;
toplevel : <fundef> | <state> | <comp> | <list> | <expr> ;
lispy    : /^/ <toplevel>* /$/ ;
listq    : <ident> '[' <expr> ']' ;
return   : "return" <expr> ';' ;

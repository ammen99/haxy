number : /-?[0-9]+/ ;
dbl    : /-?[0-9]+[.][0-9]+/ ;
bool   : "true" | "false" ;
str    : /\"(\\\\.|[^\"])*\"/ ;

ident  : /[a-zA-Z_][a-zA-Z0-9_]*/ ;
list   : '[' <args> ']' ;
listq  : <ident> ('[' <expr> ']')+ ;

value  : <member> | <gcomp> | <listq> | <str> | <func> | <dbl> | <number> | <bool> | <ident> | <list> | '(' (<op1> | <op2> | <op3> | <op4>) <expr>+ ')' ;

op4    : '*' | '/' | '%' ;
op3    : '+' | '-' ;
op2    : "==" | "<=" | ">=" | "!=" | '>' | '<' ;
op1    :  "&&" | "||" | "**" ;

comp1  : (<comp2> | <comp3> | <comp4> | <value>) <op1> (<comp1> | <comp2> | <comp3> | <comp4> | <value>);
comp2  : (<comp3> | <comp4> | <value>)           <op2> (<comp2> | <comp3> | <comp4> | <value>);
comp3  : (<comp4> | <value>)                     <op3> (<comp3> | <comp4> | <value>);
comp4  : (<value>)                               <op4> (<comp4> | <value>);

comp   : <comp1> | <comp2> | <comp3> | <comp4>;
gcomp  : '(' <comp> ')' ;
expr   : <comp> | <value> ;

noarg  : "" ;
args   : (<expr> ',')* <expr> | <noarg> ;
func   : <ident> '(' <args> ')' ;
fundef : "def" <func> <body> ;

assign : (<listq> | <ident> | <member>) '=' <expr> ;
var    : "var" (<assign> | <ident>) (',' (<assign> | <ident>))* ';' ;

if     : "if" <gcomp> <body> ;
elif   : "elif" <gcomp> <body> ;
else   : "else" <body> ;
cond   : <if> <elif>* <else>* ;
while  : "while" <gcomp> <body> ;

return : "return" <expr> ';' ;
state  : <return> | <var> | <assign> ';' | <listq> | <func> ';' | <member> ';' | <cond> | <while> ;
body   : '{' <state>* '}' ;

class   : "class" <ident> '{' (<var> | <fundef>)* '}' ;
memtype : <func> | <listq> | <ident> ;
member  : <memtype> ('@' <memtype>)+ ;

toplvl : <class> | <fundef> | <state> | <comp> | <list> | <expr> ;
haxy   : /^/ <toplvl>* /$/ ;

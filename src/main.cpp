#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>

#include "val.hpp"
#include "eval.hpp"
#include "byte.hpp"

#define LEXER_FILE "/usr/local/share/haxy/lexer/num.lex"

#define src_name(x) x + ".hx"
#define byte_name(x) "._" + x + ".hc"

std::string load_file(std::string path) {
    std::ifstream stream(path, std::ios::in);

    std::string res, line;
    while(std::getline(stream, line)) res += line + '\n';
    
    stream.close();

    return res;
}

bool file_exists(std::string str) {
    struct stat tmp;
    return (stat(str.c_str(), &tmp) == 0);
}

time_t get_last_modified_time(std::string file) {
    struct stat tmp;

    int status = stat(file.c_str(), &tmp);
    if(status != 0) return -1; 

    return tmp.st_mtime;
}

/* used to reserve compiler types only once */
struct CompilerVars {
    bool done_init = false;

    mpc_parser_t *num, *dbl, *bl, *str,
                 *id, *lst, *lq, *value,
                 *op1, *op2, *op3, *op4,
                 *comp1, *comp2, *comp3, *comp4,
                 *comp, *gcomp, *expr,
                 *noarg, *arg  , *args , *func, *fundef,
                 *var, *assign,
                 *_if, *elif, *_else, *cond, *_while,
                 *ret, *state, *body,
                 *_class, *memtype, *member,
                 *toplvl, *hax;

    void init() {
        num    = mpc_new("number"); dbl    = mpc_new("dbl");    bl      = mpc_new("bool"); 
        str    = mpc_new("str");    op1    = mpc_new("op1");    op2     = mpc_new("op2");
        op3    = mpc_new("op3");    op4    = mpc_new("op4");    comp1   = mpc_new("comp1");
        comp2  = mpc_new("comp2");  comp3  = mpc_new("comp3");  comp4   = mpc_new("comp4");
        id     = mpc_new("ident");  noarg  = mpc_new("noarg");  arg     = mpc_new("arg");
        args   = mpc_new("args");   ret    = mpc_new("return"); lq      = mpc_new("listq"); 
        func   = mpc_new("func");   expr   = mpc_new("expr");   var     = mpc_new("var");
        lst    = mpc_new("list");   state  = mpc_new("state");  fundef  = mpc_new("fundef");
        toplvl = mpc_new("toplvl"); value  = mpc_new("value");  body    = mpc_new("body");
        comp   = mpc_new("comp");   gcomp  = mpc_new("gcomp");  _if     = mpc_new("if");
        elif   = mpc_new("elif");   _while = mpc_new("while");  _else   = mpc_new("else");
        cond   = mpc_new("cond");   assign = mpc_new("assign"); hax     = mpc_new("haxy");
        _class = mpc_new("class");  member = mpc_new("member"); memtype = mpc_new("memtype");

        auto lex = load_file(LEXER_FILE);
        mpca_lang(MPCA_LANG_DEFAULT, lex.c_str(), _class, lq, ret, _while, assign,
                elif, _else, cond, _if, value, comp, gcomp, num, bl,
                op1, op2, op3, op4, comp1, comp2, comp3, comp4, comp,
                body, str, id, arg, args, noarg, func, memtype,
                expr, state, fundef, var, lst, toplvl, dbl, member, hax);
    }

    void fini() {
        mpc_cleanup(39, _else, elif, noarg, cond, num, lq, str,
                _while, op1, op2, op3, op4, comp1, comp2, comp3, comp4, id,
                assign, arg, args, bl, func, value, body,
                comp, gcomp, expr, var, lst, state, dbl, _class, member, 
                memtype, ret, fundef, _if, toplvl, hax);
    }

} mpc_machine;

#define ModeInterpret (1 << 0)
#define ModeCompile   (1 << 1)

void compile(std::string name, bool debug = false) {
    std::cout << "compiling ... " << std::endl;

    if(!file_exists(src_name(name))) {
        std::cerr << "Requested compilation of file named "
            << name + ".hx,\n which doesn't exist. Terminating..." << std::endl;
        std::exit(-1);
    }

    if(!mpc_machine.done_init) mpc_machine.init();

    std::string src = load_file(src_name(name));

    mpc_result_t res;
    if(mpc_parse("input", src.c_str(), mpc_machine.hax, &res)) {

        if(debug) mpc_ast_print(ast(res.output));

        auto tree = haxy::AstGenerator::parse_file((AstNodeT)(res.output));

        if(debug) {
            haxy::AstPrinter printer;
            printer.write_tree(tree);
        }

        haxy::AstWriter writer(byte_name(name),
                get_last_modified_time(src_name(name)));
        writer.write_tree(tree);
    }
    else {
        std::cerr << "Compilation failed! Possibly syntax error in file!" << std::endl;
        mpc_err_print(res.error),
        mpc_err_delete(res.error);
    } 
}

int main(int argc, char *argv[]) {

    extern char *optarg;
    extern int   optind;

    int mode = ModeInterpret;

    int c;

    while((c = getopt(argc, argv, "c::i::")) != -1) {
        switch (c){
            case 'c':
                mode = ModeCompile;
                break;
            case 'i':
                mode = ModeInterpret;
                break;
        }
    }

    std::string name = argv[optind];

    /* remove .hx ending if it is given */
    auto sz = name.size();
    if(name.size() > 3 &&
            name[sz - 3] == '.' &&
            name[sz - 2] == 'h' &&
            name[sz - 1] == 'x')
        name = name.substr(0, sz - 3);

    //compile(name, 1);
    //return 0;

    /* ModeInterpret reads the already generated bytecode and executes it */
    if(mode == ModeInterpret) {

        auto last_modified = get_last_modified_time(src_name(name));
        if(!file_exists(byte_name(name)))
            compile(name, false);

        auto src = load_file(byte_name(name)); 

        haxy::AstReader reader(src);

        haxy::AstNode tree;

        /* compile if bytecode file is for older version of file */
        auto timestamp = reader.get_timestamp();

        if(timestamp != last_modified) {
            compile(name, false);

            src = load_file(byte_name(name));

            haxy::AstReader reader2(src);  
            reader2.get_timestamp();

            tree = reader2.read_tree();
        }

        else
            tree = reader.read_tree();

        haxy::AstPrinter p;
        p.write_tree(tree);
        haxy::AstEvaluator evaluator;
        evaluator.init();
        evaluator.eval(tree);
    }

    else {
        compile(name, false);
    }

    if(mpc_machine.done_init)
        mpc_machine.fini();

    return 0;
}

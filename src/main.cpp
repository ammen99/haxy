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

    mpc_parser_t *num, *dbl, *bl, *str, *op, *id, *noarg, *arg  , *args , *func ,
                 *expr, *var  , *lst, *st   ,*fd   ,*top, *norm , *body ,
                 *comp, *gcomp, *iff, *elif ,*felif,*wh   , *elsee, *cond ,
                 *assi, *cmd  , *ret, *lq   ,*clss ,*memb, *memtype;

    void init() {
        num  = mpc_new("number"); dbl  = mpc_new("dbl"); bl   = mpc_new("bool");
        str  = mpc_new("str"); op   = mpc_new("operator"); id   = mpc_new("ident");
        noarg= mpc_new("noarg"); arg  = mpc_new("arg"); args = mpc_new("args");
        func = mpc_new("func"); expr = mpc_new("expr"); var  = mpc_new("var");
        lst  = mpc_new("list"); st   = mpc_new("state"); fd   = mpc_new("fundef");
        top  = mpc_new("toplevel"); norm = mpc_new("value"); body = mpc_new("body");
        comp = mpc_new("comp"); gcomp= mpc_new("gcomp"); iff  = mpc_new("if");
        elif = mpc_new("elif"); felif= mpc_new("felif"); wh   = mpc_new("while");
        elsee= mpc_new("else"); cond = mpc_new("cond"); assi = mpc_new("assign");
        cmd  = mpc_new("lispy"); ret  = mpc_new("return"); lq   = mpc_new("listq");
        clss = mpc_new("class"); memb = mpc_new("member"); memtype = mpc_new("memtype");

        auto lex = load_file(LEXER_FILE);
        mpca_lang(MPCA_LANG_DEFAULT, lex.c_str(), clss, lq, ret, wh, assi,
                felif, elif, elsee, cond, iff, norm,
                comp, gcomp, num, bl, op, body, str, id, 
                arg, args, noarg, func, memtype,
                expr, st, fd, var, lst, top, dbl, memb, cmd);
    }

    void fini() {
        mpc_cleanup(30, elsee, elif, noarg, cond, num, lq, str,
                wh, op, id, assi, felif, arg, args, bl, func, norm, body,
                comp, gcomp, expr, var, lst, st, dbl, clss, memb, 
                memtype, ret, fd, iff, top, cmd);
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
    if(mpc_parse("input", src.c_str(), mpc_machine.cmd, &res)) {

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

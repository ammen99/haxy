#include <iostream>
#include <fstream>
#include <readline/readline.h>
#include <readline/history.h>

extern "C" {
#include "mpc.h"
}

#include "val.hpp"
#include "eval.hpp"

#define forever while(1)
std::string load_file(std::string path) {
    std::fstream stream(path, std::ios::in | std::ios::out);

    std::string res, line;
    while(std::getline(stream, line)) res += line + '\n';

    return res;
}



int main(int argc, char *argv[]) {

    Evaluator eval;
    eval.create_new_scope("__global__");
    eval.add_builtin_functions();

    mpc_parser_t* num  = mpc_new("number");
    mpc_parser_t* dbl  = mpc_new("dbl");
    mpc_parser_t* bl   = mpc_new("bool");
    mpc_parser_t* str  = mpc_new("str");
    mpc_parser_t* op   = mpc_new("operator");
    mpc_parser_t* id   = mpc_new("ident");
    mpc_parser_t* noarg= mpc_new("noarg");
    mpc_parser_t* arg  = mpc_new("arg");
    mpc_parser_t* args = mpc_new("args");
    mpc_parser_t* func = mpc_new("func");
    mpc_parser_t* expr = mpc_new("expr");
    mpc_parser_t* var  = mpc_new("var");
    mpc_parser_t* lst  = mpc_new("list");
    mpc_parser_t* st   = mpc_new("state"); 
    mpc_parser_t* fd   = mpc_new("fundef");
    mpc_parser_t* top  = mpc_new("toplevel");
    mpc_parser_t* norm = mpc_new("normal");
    mpc_parser_t* body = mpc_new("body");
    mpc_parser_t* comp = mpc_new("comp");
    mpc_parser_t* gcomp= mpc_new("gcomp");
    mpc_parser_t* iff  = mpc_new("if");
    mpc_parser_t* elif = mpc_new("elif");
    mpc_parser_t* felif= mpc_new("felif");
    mpc_parser_t* wh   = mpc_new("while");
    mpc_parser_t* elsee= mpc_new("else");
    mpc_parser_t* cond = mpc_new("cond");
    mpc_parser_t* assi = mpc_new("assign");
    mpc_parser_t* cmd  = mpc_new("lispy");

    auto lex = load_file("/home/ilex/work/lispy/src/num.lex");
    mpca_lang(MPCA_LANG_DEFAULT, lex.c_str(), wh, assi, felif, elif, elsee, cond, iff, norm,
            comp, gcomp, num, bl, op, body, str, id, 
            arg, args, noarg, func,
            expr, st, fd, var, lst, top, dbl, cmd);

    //mpca_lang(MPCA_LANG_DEFAULT, lex.c_str());
    if(argc < 2) { /* run interactive mode */
        try {

            forever {
                std::string input = readline("$>");
                add_history(input.c_str());

                mpc_result_t res;
                if(mpc_parse("input", input.c_str(), cmd, &res)) {
                    mpc_ast_print(ast(res.output)); 
                    //eval.eval(ast(res.output));
                    std::cout << eval.eval(ast(res.output)) << std::endl;
                    mpc_ast_delete(ast(res.output));
                }
                else {
                    mpc_err_print(res.error);
                    mpc_err_delete(res.error);
                }
            }

        } catch (std::logic_error e) { /* user has pressed <C-d> => quit */
            std::cout << "\nGoodbye!" << std::endl; 
        }    
    }

    else {
        std::string s = load_file(argv[1]);
        mpc_result_t res;

        if(mpc_parse("input", s.c_str(), cmd, &res)) {
            mpc_ast_print(ast(res.output)); 
            eval.eval_file(ast(res.output));
            mpc_ast_delete(ast(res.output));
        }
        else
            mpc_err_print(res.error),
            mpc_err_delete(res.error);
    }

    mpc_cleanup(9, elsee, elif, noarg, cond, num, str, wh, op, id, assi, felif, arg, args, bl, func, norm, body,
            comp, gcomp, expr, var, lst, st, dbl, fd, iff, top, cmd);
    return 0;
}

#ifndef EVAL_H
#define EVAL_H

#include "val.hpp"
extern "C" {
#include "mpc.h"
}
#include <unordered_map>
#include <string>
#include <cstring>
#include <vector>
#include <functional>
#define ast(x) (mpc_ast_t*)(x)
std::string load_file(std::string name);
using AstNodeT = mpc_ast_t*;
class Evaluator { 
    struct Scope {
        std::unordered_map<std::string, Value> vars;
        std::unordered_map<std::string, Func> funcs;
        std::string name;
        Scope *parent_scope = nullptr;
    } *current_scope = nullptr;

    mpc_parser_t *parser, *expr_parser;

    std::vector<mpc_ast_t*> loaded_asts;

    void  new_var(std::string name, Value val);
    void  set_var(std::string name, Value val);
    const Value& get_var(std::string name);

    Func get_func(std::string name);
    void new_func(std::string name, AstNodeT node);
    void new_func(std::string name, Func f);


    void pop_scope();
    void print_trace();

    Value eval_int(std::string str);
    Value eval_bool(std::string str);

    List  eval_list(AstNodeT node);

    /* evaluate args(normal) */
    Args  eval_args(AstNodeT node);
    /* evaluate args by identifiers(do not lookup for variables),
     * see implementation for explanation */
    Args  eval_args_identifiers(AstNodeT node);


    Value call_func(AstNodeT code, Args args);
    Value eval_func(AstNodeT node);
    Value eval_comp(AstNodeT node);

    /* evaluates a series of instructions
     * and can introduce a new scope */
    Value eval_block(AstNodeT node, std::string new_scope = "__unnamed__");

    Value eval_if(AstNodeT node);

    Value eval_while(AstNodeT node);

    public:
    void add_builtin_functions();
    void create_new_scope(std::string name);

    void init(mpc_parser_t *parser, mpc_parser_t *expr_parser);
    void cleanup();

    void eval_file(std::string name);
    Value eval(AstNodeT node);
    bool check_args(Args a);
};


#endif /* end of include guard: EVAL_H */


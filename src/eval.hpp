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

using AstNode = mpc_ast_t*;

class Evaluator {

    struct Scope {
        std::unordered_map<std::string, Value> vars;
        std::unordered_map<std::string, Func> funcs;

        std::string name;
        Scope *parent_scope = nullptr;
    } *current_scope = nullptr;

    mpc_parser_t *parser;
    std::vector<mpc_ast_t*> loaded_asts;

    void  new_var(std::string name, Value val);
    void  set_var(std::string name, Value val);
    Value get_var(std::string name);

    Func get_func(std::string name);
    void new_func(std::string name, AstNode node);
    void new_func(std::string name, Func f);


    void pop_scope();

    Value eval_int(std::string str);
    Value eval_bool(std::string str);

    List  eval_list(AstNode node);

    /* evaluate args(normal) */
    Args  eval_args(AstNode node);
    /* evaluate args by identifiers(do not lookup for variables),
     * see implementation for explanation */
    Args  eval_args_identifiers(AstNode node);


    Value call_func(AstNode code, Args args);
    Value eval_func(AstNode node);
    Value eval_comp(AstNode node);

    /* evaluates a series of instructions
     * does not return a value
     * introduces a new scope */
    void eval_block(AstNode node);

    bool eval_if(AstNode node);
    bool eval_elif(AstNode node);
    void eval_else(AstNode node);

    void eval_while(AstNode node);

    public:
    void add_builtin_functions();
    void create_new_scope(std::string name);

    void init(mpc_parser_t *parser);
    void cleanup();

    void eval_file(std::string name);
    Value eval(AstNode node);
    bool check_args(Args a);
};


#endif /* end of include guard: EVAL_H */


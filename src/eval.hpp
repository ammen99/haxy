#ifndef EVAL_H
#define EVAL_H
extern "C" {
#include "mpc.h"
}

#include <string>
#include <cstring>
#include <vector>
#include <functional>

#include "val.hpp"
#include "ast.hpp"

namespace haxy {

class AstEvaluator { 
    Scope *current_scope = nullptr;

    mpc_parser_t *parser, *expr_parser;

    void   new_var(std::string name, Value val);
    void   set_var(std::string name, Value val);
    Value  get_var(std::string name);

    Func get_func(std::string name);

    void new_func(AstNode node);
    void create_constructor(AstNode node);
    void new_func(std::string name, Func f);


    void pop_scope();
    void print_trace();

    Value eval_int(std::string str);
    Value eval_bool(std::string str);

    List  eval_list(AstNode node);

    /* evaluate args */
    Args  eval_args(const std::vector<AstNode>& args, bool by_ident = false);


    Value call_func(AstFunctionDefinition code, Args args);
    Value eval_func(AstNode node);
    Value eval_comp(AstNode node);

    /* evaluates a series of instructions
     * and can introduce a new scope */
    Value eval_block(AstBlock node, std::string new_scope = "__unnamed__", bool create = true);

    Value eval_listq(AstListQ node);
    Value eval_classref(AstClassReference node);
    Value eval_assignment(AstAssignment node);
    Value eval_vardecl(AstVariableDeclaration node);

    Value eval_if(AstNode node);
    Value eval_while(AstNode node);

    public:
    void add_builtin_functions();
    void create_new_scope(std::string name);

    void init(mpc_parser_t *parser, mpc_parser_t *expr_parser);

    void eval_file(std::string name);
    Value eval(AstNode node);
    bool check_args(Args a);
};
}

#endif /* end of include guard: EVAL_H */


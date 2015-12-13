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
    struct ScopeStack {
        private:
        std::vector<Scope> stack;

        public:
        Value get_var(std::string name);
        void  new_var(std::string name, Value val);

        Func get_func(std::string name);
        void new_func(std::string name, Func f);
    
        void  push_scope(Scope s); /* push an existing scope */
        void  push_scope(std::string name); /* create and push a scope */
        void  pop_scope(); /* remove scope */
        Scope get_top_scope(); /* returns the last scope */

        void print_trace();
    } scope_stack;

    mpc_parser_t *parser, *expr_parser;

    void new_func(AstNode node);
    void create_constructor(AstNode node);

    List  eval_list(AstNode node);

    /* evaluate args */
    Args  eval_args(const std::vector<AstNode>& args, bool by_ident = false);


    Value function_call_wrapper(AstFunctionDefinition code, Args args);


    Value call_function(AstNode node); /* function is in current scope */
    Value call_function(Func fn, AstFunctionCall node); /* function is not in current scope */

    /* evaluates a series of instructions
     * and can introduce a new scope */
    Value eval_block(AstBlock node, std::string new_scope = "__unnamed__", bool create = true);

    Value eval_listq(Value lst, AstListQ node); /* used when the list is not in current scope */
    Value eval_listq(AstListQ node); /* evaluate listq when the list is in current scope */

    Value eval_classref(AstClassReference node);
    Value eval_assignment(AstAssignment node);
    Value eval_vardecl(AstVariableDeclaration node);

    Value eval_if(AstNode node);
    Value eval_while(AstNode node);

    public:
    void add_builtin_functions();
    void init(mpc_parser_t *parser, mpc_parser_t *expr_parser);

    void eval_file(std::string name);
    Value eval(AstNode node);
    bool check_args(Args a);
};
}

#endif /* end of include guard: EVAL_H */


#ifndef EVAL_H
#define EVAL_H

#include <string>
#include <cstring>
#include <vector>
#include <functional>
#include <stack>
#include "val.hpp"
#include "ast.hpp"

namespace haxy {

    enum ScopeType {
        ScopeTypePrimary    = 1,
        ScopeTypeDerived    = 2,
        ScopeTypeNonderived = 3


class AstEvaluator { 
    struct ScopeStack {
        struct CallScope {
            Scope scope;
            int prev_jump = -1;
        };

        private:
        std::vector<CallScope> stack;
        std::stack<int> last_primary;
        std::stack<int> last_nonderived;

        public:
        Value get_var(std::string name);
        void  new_var(std::string name, Value val);

        Func get_func(std::string name);
        void new_func(std::string name, Func f);
    
        void  push_scope(Scope s, ScopeType type); /* push an existing scope */
        void  push_scope(std::string name, ScopeType type); /* create and push a scope */
        void  pop_scope(); /* remove scope */
        Scope get_top_scope(); /* returns the last scope */

        void print_trace();
    } scope_stack;

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
    void init();

    void eval_file(std::string name);
    Value eval(AstNode node);
    bool check_args(Args a);
};
}

#endif /* end of include guard: EVAL_H */


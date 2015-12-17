#include "eval.hpp"

#define donotcreatescope "________________________"

namespace haxy {
void AstEvaluator::init() {
    scope_stack.push_scope("__global__", ScopeTypePrimary);
    add_builtin_functions();
}

/* definitions for Scope */

void AstEvaluator::ScopeStack::push_scope(std::string name, ScopeType type) {
    std::cout << "push scope " << name << " " << type << std::endl;
    CallScope new_scope;
    new_scope.scope.name = name;

    switch(type) {
        case ScopeTypePrimary:
            if(!last_primary.empty())
                new_scope.prev_jump = last_primary.top() - stack.size();
            last_primary.push(stack.size());
            break;

        case ScopeTypeNonderived:
            new_scope.prev_jump = last_primary.top() - stack.size();
            break;

        default:
            break;
    }

    stack.push_back(new_scope);
}

void AstEvaluator::ScopeStack::push_scope(Scope s, ScopeType type) {
    CallScope new_scope;
    new_scope.scope = s;

    switch(type) {
        case ScopeTypePrimary:
            if(!last_primary.empty())
                new_scope.prev_jump = last_primary.top() - stack.size();
            last_primary.push(stack.size());
            break;

        case ScopeTypeNonderived:
            new_scope.prev_jump = last_primary.top() - stack.size();
            break;

        default:
            break;
    }   

    stack.push_back(new_scope);
}

void AstEvaluator::ScopeStack::pop_scope() {
    if(stack.empty()) return;
    if(stack.size() - 1 == (last_primary.empty() ? -1 : last_primary.top()))
        last_primary.pop();
    stack.pop_back();
}

Scope AstEvaluator::ScopeStack::get_top_scope() {
    if(stack.empty()) return Scope();

    return stack[stack.size() - 1].scope;
}

void AstEvaluator::ScopeStack::print_trace() {
    std::string res;

    for(int i = 0; i < stack.size(); i++) {
        for(int j = 0; j < i * 2; j++) std::cout << " "; 

        std::cout << stack[i].prev_jump << " " << stack[i].scope.name << std::endl;;
    }
}

void AstEvaluator::ScopeStack::new_var(std::string name, Value val) {
    stack[stack.size() - 1].scope.vars[name] = val;
}

Value unknownsym = new_error(UnknownSym);
Value badvalue   = new_error(BadValue);

Value AstEvaluator::ScopeStack::get_var(std::string name) {
    int i = stack.size() - 1;

    while( i >= 0 ) {
        auto it = stack[i].scope.vars.find(name);
        if(it != stack[i].scope.vars.end()) return it->second;
        i += stack[i].prev_jump;
    }

    return unknownsym;
}

void AstEvaluator::ScopeStack::new_func(std::string name, Func f) {
    stack[stack.size() - 1].scope.funcs[name] = Func(new _Func{f->min_arg, f->max_arg, [=] (Args a) {
        push_scope(name + "()", ScopeTypeNonderived); 
        Value v = f->call(a);
        pop_scope();
        return v;
    }, f->eval_args_by_identifier});
}

Func AstEvaluator::ScopeStack::get_func(std::string name) {

    int i = stack.size() - 1;

    while( i >= 0 ) {
        auto it = stack[i].scope.funcs.find(name);
        if(it != stack[i].scope.funcs.end()) return it->second;
        i += stack[i].prev_jump;
    }
    return Func(new _Func{0, 0, [] (Args a) {return new_error(UnknownSym);}});
}

/* End of ScopeStack */


void AstEvaluator::new_func(AstNode node) {
    auto func = node.convert<_AstFunctionDefinition>();

    scope_stack.new_func(func->name,
            Func(new _Func{0, MAXINT,
            [=] (Args a) {
                return function_call_wrapper(func, a);
            },
            false}));
}


void AstEvaluator::create_constructor(AstNode node) {
    std::cout << "create con" << std::endl;
    auto cl = node.convert<_AstClass>();

    scope_stack.new_func(cl->name, Func(new _Func{0, 0, [=] (Args a) {
                Value v;
                v->type = ValueTypeScope;

                scope_stack.push_scope(cl->name, ScopeTypeNonderived);

                for(auto fun : cl->funcs)
                    new_func(fun);
                
                for(auto var : cl->vars)
                    eval_vardecl(var);

                new(&v->sc) Scope{scope_stack.get_top_scope()};
                scope_stack.pop_scope();
                return v;
            }}));
}

Value AstEvaluator::function_call_wrapper(AstFunctionDefinition node, Args a) {
    for(int i = 0; i < node->args.size() && i < a.size(); i++)
        scope_stack.new_var(node->args[i], a[i]);

    Value v = eval_block(node->action);
    v->return_value = false;

    return v;
}

/* end scope */


void AstEvaluator::add_builtin_functions() {
    
    scope_stack.new_func("sum", Func(new _Func{2, MAXINT, [=] (Args a) {
        Value s = new_value(0);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s->long_val += x->long_val; 

        return s;
    }, false}));

    scope_stack.new_func("product", Func(new _Func{2, MAXINT, [=] (Args a) {
        Value s = new_value(1);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s->long_val *= x->long_val;

        return s;
    }, false}));

    scope_stack.new_func("sum_range", Func(new _Func{2, 2, [=] (Args a) {
        if(!check_args(a)) return new_error(BadValue);

        int start = a[0]->long_val;
        int end = a[1]->long_val;

        return new_value(end * (end + 1) / 2 - (start - 1) * (start) / 2);
    }, false}));

    scope_stack.new_func("product_range", Func(new _Func{2, 2, [=] (Args a) {
        Value v = new_value(1); 

        if(!check_args(a)) return new_error(BadValue);

        int start = a[0]->long_val;
        int end = a[1]->long_val;

        for(int i = start; i <= end; i++) {
            v->long_val *= i;
        }

        return v;
    }, false}));

    scope_stack.new_func("print", Func(new _Func{0, MAXINT, [=] (Args a) {
        for(int i = 0; i < a.size(); i++)
            std::cout << a[i] << " ";
        std::cout << std::endl;
        return new_error(NoValue); 
    }, false}));

    scope_stack.new_func("trace", Func(new _Func{0, 0, [=] (Args a) {
            scope_stack.print_trace();
            return new_error(NoValue);
            }, false}));


    // TODO: implement readln function
//    scope_stack.new_func("readln", Func(new _Func{1, MAXINT, [=] (Args a) {
//        for(int i = 0; i < a.size(); i++) {
//            std::string str;
//            std::getline(std::cin, str);
//
//            mpc_result_t res;
//
//            if(mpc_parse("input", str.c_str(), expr_parser, &res)) {
//                mpc_ast_print(ast(res.output)); 
//                auto node = ast(res.output);
//                Value v;
//                if(node->tag == std::string("ident|regex"))
//                    v = new_value(node->contents);
//                else
//                    v = eval(node);
//
//                set_var(a[i]->str, v);
//          }
//            else {
//                mpc_err_print(res.error),
//                mpc_err_delete(res.error);
//            }
//                 
//        }
//        return new_error(NoValue);        
//    }, true}));

    scope_stack.new_func("read", Func(new _Func{1, MAXINT, [=] (Args a) {
            for(int i = 0; i < a.size(); i++) {
                
            }
            return new_error(UnknownSym); 
    }, true}));

    scope_stack.new_func("import", Func(new _Func{1, 1, [=] (Args a) {

        if(a[0]->type != ValueTypeString)
            return new_error(BadValue);

        eval_file(a[0]->str + ".hx");
        return new_error(NoValue); 
    }, true}));
}

/* different functions for evaluating */

/* does and operation expression */
Value doit(Value a, std::string o, Value b) {

    if(o == "+") return a + b;
    if(o == "-") return a - b;
    if(o == "*") return a * b;
    if(o == "/") return a / b;
    if(o == "%") return a % b;

    if(o == "||") return a || b;
    if(o == "&&") return a && b;

    if(o == "==") return a == b;
    if(o == ">=") return a >= b;
    if(o == "<=") return a <= b;
    if(o == "!=") return a != b;
    if(o == ">")  return a > b;
    if(o == "<")  return a < b;
    if(o == "**") return times(a, b);

    return new_error(BadOp);
}

bool AstEvaluator::check_args(Args a) {
    for(auto arg : a)
        if(arg->type == ValueTypeError) return false;

    return true;
}


Value AstEvaluator::call_function(Func fn, AstFunctionCall node) {
    Args args;

    if(fn->eval_args_by_identifier)
        for(auto x : node->args)
            args.push_back(new_value(x.convert<_AstVariable>()->name));
    else
        for(auto x : node->args)
            args.push_back(eval(x));

    if(fn->min_arg <= args.size() && args.size() <= fn->max_arg)
        return fn->call(args);
    else
        return new_error(ArgumentNumberMismatch);
    
}

Value AstEvaluator::call_function(AstNode node) {
    auto call = node.convert<_AstFunctionCall>();
    return call_function(scope_stack.get_func(call->name), call);
}

Value AstEvaluator::eval_if(AstNode nnode) {

    auto node = nnode.convert<_AstConditional>();
    for(auto x : node->children) {

        switch(x->type) {
            case IfTypeElse:
                return eval_block(x->block, "else"); 
                break;

            default:
                Value res = eval(x->expr);

                if((res->type & ValueTypeBoolArithm) && res->long_val) {
                    return eval_block(x->block, "if");
                }
                else if (res->type == ValueTypeError) {
                    std::cout << "error\n";
                    res->return_value = true;
                    return res;
                }
        }
    }

    return new_value(NoValue);;
}

Value AstEvaluator::eval_while(AstNode nnode) {
    auto node = nnode.convert<_AstWhile> ();
    Value val;

    while((val = eval(node->condition))->to_bool()) {
        Value tmp = eval_block(node->action, "while");
        if(tmp->return_value) return tmp;
    }

    return new_error(NoValue);
}

Value AstEvaluator::eval_listq(Value arr, AstListQ listq) {
    for(auto index : listq->indices) {
        Value ind_val = eval(index);
        if(ind_val->type != ValueTypeNumber || arr->type != ValueTypeList)
            return badvalue;

        auto size = arr->lst.size();
        auto ind = ind_val->long_val;

        if(ind < 0 || ind > size)
            ind_val->long_val = (ind % size + size) % size;

        arr = (*arr)[ind_val];
    }

    return arr;
}

Value AstEvaluator::eval_listq(AstListQ listq) {
    return eval_listq(scope_stack.get_var(listq->name), listq);
}

Value AstEvaluator::eval_classref(AstClassReference clref) {
    std::cout << "eval classref " << clref->refs.size() << std::endl;
    Value arr = eval(clref->refs[0]);;

    for(int i = 1; i < clref->refs.size(); i++) {

        if(arr->type != ValueTypeScope) return badvalue; 
        Scope& sc = arr->sc;

        switch(clref->refs[i]->tag) {
            case AstTagVariable: {
                auto name = clref->refs[i].convert<_AstVariable>()->name; 

                auto it = sc.vars.find(name);
                if(it == sc.vars.end()) 
                    return unknownsym;

                arr = it->second;
                break;
            }

            case AstTagListQ: {
                auto listq = clref->refs[i].convert<_AstListQ>();                  

                auto it = sc.vars.find(listq->name);
                if(it == sc.vars.end()) return unknownsym;

                arr = eval_listq(it->second, listq);
                break;
            }

            case AstTagFunctionCall: {

                scope_stack.push_scope(sc, ScopeTypePrimary); 

                arr = call_function(clref->refs[i]);

                scope_stack.pop_scope();
                break;
            }

            default:
                break;

        }
    }

    return arr;
}

Value AstEvaluator::eval_assignment(AstAssignment node) {

    Value val = eval(node->right_side);

    switch(node->left_side->tag) {
        case AstTagListQ: {
            Value left = eval_listq(node->left_side.convert<_AstListQ>());
            *left = *val;
            break;
        }

        case AstTagVariable: {
            Value left = scope_stack.get_var(node->left_side.convert<_AstVariable>()->name);
            if(left->type == ValueTypeError && left->error == UnknownSym) return unknownsym;

            *left = *val;
            break;
        }

        default:
            break;
    }

    return val;
}

Value AstEvaluator::eval_vardecl(AstVariableDeclaration node) {
    for(int i = 0; i < node->children.size(); i++) {
        switch(node->children[i]->tag) {
            case AstTagAssignment: {
                auto var = node->children[i].convert<_AstAssignment>();

                if(var->left_side->tag != AstTagVariable) {
                    std::cout << "Cannot declare non-variables" << std::endl;
                    continue;
                }

                scope_stack.new_var(var->left_side.convert<_AstVariable>()->name, new_error(NoValue));
                eval_assignment(var);

                break;
            }

            case AstTagVariable:
                scope_stack.new_var(node->children[i].convert<_AstVariable>()->name, new_error(NoValue));
                break;

            default:
                break;
        }
    }

    return new_error(NoValue);
}


Value AstEvaluator::eval(AstNode node) {
    switch(node->tag) {

        case AstTagValue:
            return node.convert<_AstValue>()->value;

        case AstTagList: {
            auto lstnode = node.convert<_AstList>();

            List lst;
            for(auto x : lstnode->elems)
                lst.push_back(eval(x));

            return new_value(lst);
        }

        case AstTagVariable:
            return scope_stack.get_var(node.convert<_AstVariable>()->name);

        case AstTagOperation: {
            auto op = node.convert<_AstOperation>();
            return doit(eval(op->left), op->op, eval(op->right));
        }

        case AstTagFoldExpression: {
            auto expr = node.convert<_AstFoldExpr>();
            Value x = eval(expr->args[0]);

            for(int i = 1; i < expr->args.size(); i++)
                x = doit(x, expr->op, eval(expr->args[i])); 

            return x;
        }

        case AstTagAssignment:
            return eval_assignment(node.convert<_AstAssignment>()); 

        case AstTagVariableDeclaration:
            return eval_vardecl(node.convert<_AstVariableDeclaration>());

        case AstTagListQ: 
            return eval_listq(node.convert<_AstListQ>());

        case AstTagConditional:
            return eval_if(node);
        
        case AstTagWhile:
            return eval_while(node);

        case AstTagFunctionDefinition : {
            new_func(node);
            return new_error(NoValue);
        }

        case AstTagFunctionCall:
            return call_function(node);

        case AstTagReturn: {
            Value v = eval(node.convert<_AstReturn>()->expr);
            v->return_value = true;
            return v;
        }
        case AstTagBlock:
            return eval_block(node.convert<_AstBlock>(), donotcreatescope, false);

        case AstTagClass:
            create_constructor(node);
            break;

        case AstTagClassRef:
            return eval_classref(node.convert<_AstClassReference>());

        default:
            std::cout << "Hit eval(AstTagBlock/Expr). This means bug!" << std::endl;
    }

    return new_error(NoValue);
}

Value AstEvaluator::eval_block(AstBlock node, std::string new_scope, bool create_scope) {
    if(create_scope) scope_stack.push_scope(new_scope, ScopeTypeDerived);

    for(int i = 0; i < node->children.size(); i++) {
        Value v = eval(node->children[i]);
        if(v->return_value) {
            if(create_scope) scope_stack.pop_scope();
            return v;
        }
    }

    if(create_scope) scope_stack.pop_scope();
    return new_error(NoValue);
}

// TODO: implement eval_file(used for import) */
void AstEvaluator::eval_file(std::string name) {
}
}
/* end eval_xxx */

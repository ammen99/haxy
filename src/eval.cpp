#include "eval.hpp"

#define donotcreatescope "________________________"

namespace haxy {
void AstEvaluator::init(mpc_parser_t *p, mpc_parser_t *e) {
    this->parser = p;
    this->expr_parser = e;

    scope_stack.push_scope("__global__");
    add_builtin_functions();
}

/* definitions for Scope */

void AstEvaluator::ScopeStack::push_scope(std::string name) {
    Scope new_scope;
    new_scope.name = name;
    stack.push_back(new_scope);
}

void AstEvaluator::ScopeStack::push_scope(Scope s) {
    stack.push_back(s);
}

void AstEvaluator::ScopeStack::pop_scope() {
    if(stack.empty()) return;
    stack.pop_back();
}

Scope AstEvaluator::ScopeStack::get_top_scope() {
    if(stack.empty()) return Scope();

    return stack[stack.size() - 1];
}

void AstEvaluator::ScopeStack::print_trace() {
    std::string res;

    for(int i = 0; i < stack.size(); i++) {
        for(int j = 0; j < i * 4; j++) std::cout << " "; 

        std::cout << stack[i].name << std::endl;;
    }
}

void AstEvaluator::ScopeStack::new_var(std::string name, Value val) {
    stack[stack.size() - 1].vars[name] = val;
}

Value unknownsym = new_error(UnknownSym);
Value badvalue   = new_error(BadValue);

Value AstEvaluator::ScopeStack::get_var(std::string name) {

    for(int i = stack.size() - 1; i >= 0; i--) {
        auto it = stack[i].vars.find(name);
        if(it != stack[i].vars.end()) return it->second;
    }

    return unknownsym;
}

void AstEvaluator::ScopeStack::new_func(std::string name, Func f) {
    stack[stack.size() - 1].funcs[name] = Func{f.min_arg, f.max_arg, [=] (Args a) {
        push_scope(name + "()"); 
        Value v = f.call(a);
        pop_scope();
        return v;
    }, f.eval_args_by_identifier};
}

Func AstEvaluator::ScopeStack::get_func(std::string name) {

    for(int i = stack.size() - 1; i >= 0; i--) {
        auto it = stack[i].funcs.find(name);
        if(it != stack[i].funcs.end()) return it->second;
    }

    return Func{0, 0, [] (Args a) {return new_error(UnknownSym);}};
}

/* End of ScopeStack */


void AstEvaluator::new_func(AstNode node) {
    auto func = convert<_AstFunctionDefinition>(node);

    scope_stack.new_func(func->name,
            Func{0, MAXINT,
            [=] (Args a) {
                return function_call_wrapper(func, a);
            },
            false});
}


void AstEvaluator::create_constructor(AstNode nnode) {
    auto cl = convert<_AstClass>(nnode);

    scope_stack.new_func(cl->name, {0, 0, [=] (Args a) {
                Value v;
                v->type = ValueTypeScope;

                scope_stack.push_scope(cl->name);

                for(auto fun : cl->funcs)
                    new_func(fun);
                
                for(auto var : cl->vars)
                    eval_vardecl(var);

                new(&v->sc) Scope{scope_stack.get_top_scope()};
                scope_stack.pop_scope();
                return v;
            }});
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
    
    scope_stack.new_func("sum", {2, MAXINT, [=] (Args a) {
        Value s = new_value(0);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s->long_val += x->long_val; 

        return s;
    }, false});

    scope_stack.new_func("product", {2, MAXINT, [=] (Args a) {
        Value s = new_value(1);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s->long_val *= x->long_val;

        return s;
    }, false});

    scope_stack.new_func("sum_range", {2, 2, [=] (Args a) {
        if(!check_args(a)) return new_error(BadValue);

        int start = a[0]->long_val;
        int end = a[1]->long_val;

        return new_value(end * (end + 1) / 2 - (start - 1) * (start) / 2);
    }, false});

    scope_stack.new_func("product_range", {2, 2, [=] (Args a) {
        Value v = new_value(1); 

        if(!check_args(a)) return new_error(BadValue);

        int start = a[0]->long_val;
        int end = a[1]->long_val;

        for(int i = start; i <= end; i++) {
            v->long_val *= i;
        }

        return v;
    }, false});

    scope_stack.new_func("print", {0, MAXINT, [=] (Args a) {
        for(int i = 0; i < a.size(); i++)
            std::cout << a[i] << " ";
        std::cout << std::endl;
        return new_error(NoValue); 
    }, false});

    scope_stack.new_func("trace", {0, 0, [=] (Args a) {
            scope_stack.print_trace();
            return new_error(NoValue);
            }, false});


    // TODO: implement readln function
    scope_stack.new_func("readln", {1, MAXINT, [=] (Args a) {
        for(int i = 0; i < a.size(); i++) {
            std::string str;
            std::getline(std::cin, str);

            mpc_result_t res;

            if(mpc_parse("input", str.c_str(), expr_parser, &res)) {
                mpc_ast_print(ast(res.output)); 
//                auto node = ast(res.output);
//                Value v;
//                if(node->tag == std::string("ident|regex"))
//                    v = new_value(node->contents);
//                else
//                    v = eval(node);
//
//                set_var(a[i]->str, v);
            }
            else {
                mpc_err_print(res.error),
                mpc_err_delete(res.error);
            }
                 
        }
        return new_error(NoValue);        
    }, true});

    scope_stack.new_func("read", {1, MAXINT, [=] (Args a) {
            for(int i = 0; i < a.size(); i++) {
                
            }
            return new_error(UnknownSym); 
    }, true});

    scope_stack.new_func("import", {1, 1, [=] (Args a) {

        if(a[0]->type != ValueTypeString)
            return new_error(BadValue);

        eval_file(a[0]->str + ".hx");
        return new_error(NoValue); 
    }, true});
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

    if(fn.eval_args_by_identifier)
        for(auto x : node->args)
            args.push_back(new_value(convert<_AstVariable>(x)->name));
    else
        for(auto x : node->args)
            args.push_back(eval(x));

    if(fn.min_arg <= args.size() && args.size() <= fn.max_arg)
        return fn.call(args);
    else
        return new_error(ArgumentNumberMismatch);
    
}

Value AstEvaluator::call_function(AstNode node) {
    auto call = convert<_AstFunctionCall>(node);
    return call_function(scope_stack.get_func(call->name), call);
}

Value AstEvaluator::eval_if(AstNode nnode) {

    auto node = convert<_AstConditional>(nnode);
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
    auto node = convert<_AstWhile> (nnode);
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
                auto name = convert<_AstVariable>(clref->refs[i])->name; 

                auto it = sc.vars.find(name);
                if(it == sc.vars.end()) 
                    return unknownsym;

                arr = it->second;
                break;
            }

            case AstTagListQ: {
                auto listq = convert<_AstListQ>(clref->refs[i]);                  

                auto it = sc.vars.find(listq->name);
                if(it == sc.vars.end()) return unknownsym;

                arr = eval_listq(it->second, listq);
                break;
            }

            case AstTagFunctionCall: {
                auto fcall = convert<_AstFunctionCall>(clref->refs[i]);
                auto it = sc.funcs.find(fcall->name); 
                if(it == sc.funcs.end()) return unknownsym;

                arr = call_function(it->second, fcall);
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
            Value left = eval_listq(convert<_AstListQ>(node->left_side));
            *left = *val;
            break;
        }

        case AstTagVariable: {
            auto name = convert<_AstVariable>(node->left_side)->name;

            Value left = scope_stack.get_var(convert<_AstVariable>(node->left_side)->name);
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
                auto var = convert<_AstAssignment>(node->children[i]);

                if(var->left_side->tag != AstTagVariable) {
                    std::cout << "Cannot declare non-variables" << std::endl;
                    continue;
                }

                scope_stack.new_var(convert<_AstVariable>(var->left_side)->name, new_error(NoValue));
                eval_assignment(var);

                break;
            }

            case AstTagVariable:
                scope_stack.new_var(convert<_AstVariable>(node->children[i])->name, new_error(NoValue));
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
            return convert<_AstValue>(node)->value;

        case AstTagList: {
            auto lstnode = convert<_AstList>(node);

            List lst;
            for(auto x : lstnode->elems)
                lst.push_back(eval(x));

            return new_value(lst);
        }

        case AstTagVariable:
            return scope_stack.get_var(convert<_AstVariable>(node)->name);

        case AstTagOperation: {
            auto op = convert<_AstOperation>(node);
            return doit(eval(op->left), op->op, eval(op->right));
        }

        case AstTagFoldExpression: {
            auto expr = convert<_AstFoldExpr>(node);
            Value x = eval(expr->args[0]);

            for(int i = 1; i < expr->args.size(); i++)
                x = doit(x, expr->op, eval(expr->args[i])); 

            return x;
        }

        case AstTagAssignment:
            return eval_assignment(convert<_AstAssignment>(node)); 

        case AstTagVariableDeclaration:
            return eval_vardecl(convert<_AstVariableDeclaration>(node));

        case AstTagListQ: 
            return eval_listq(convert<_AstListQ>(node));

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
            Value v = eval(convert<_AstReturn>(node)->expr);
            v->return_value = true;
            return v;
        }
        case AstTagBlock:
            return eval_block(convert<_AstBlock>(node), donotcreatescope, false);

        case AstTagClass:
            create_constructor(node);
            break;

        case AstTagClassRef:
            return eval_classref(convert<_AstClassReference>(node));

        default:
            std::cout << "Hit eval(AstTagBlock/Expr). This means bug!" << std::endl;
    }

    return new_error(NoValue);
}

Value AstEvaluator::eval_block(AstBlock node, std::string new_scope, bool create_scope) {
    if(create_scope) scope_stack.push_scope(new_scope);

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

void AstEvaluator::eval_file(std::string name) {
    std::string src = load_file(name);

    mpc_result_t res;

    if(mpc_parse("input", src.c_str(), parser, &res)) {
        mpc_ast_print(ast(res.output)); 

        auto output = convert<_AstBlock>(AstGenerator::parse_file(ast(res.output)));

        eval_block(output);
    }
    else {
        mpc_err_print(res.error),
        mpc_err_delete(res.error);
    }
}
}
/* end eval_xxx */

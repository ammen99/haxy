#include "eval.hpp"

#define unknownsym "___afx_unknown_sym___"
#define donotcreatescope "________________________"

namespace haxy {
void AstEvaluator::init(mpc_parser_t *p, mpc_parser_t *e) {
    this->parser = p;
    this->expr_parser = e;

    create_new_scope("__global__");
    //new_var(unknownsym, new_error(UnknownSym));
    add_builtin_functions();
}

/* definitions for Scope */

void AstEvaluator::create_new_scope(std::string name) {
    Scope *new_scope = new Scope;
    new_scope->name = name;
    new_scope->parent_scope = current_scope;

    current_scope = new_scope;
}

void AstEvaluator::pop_scope() {
    if(!current_scope) return;
    auto parent_scope = current_scope->parent_scope;

    delete current_scope;
    current_scope = parent_scope;
}

void AstEvaluator::print_trace() {
    std::string res;

    auto cscope = current_scope;

    while(cscope) {
        res = cscope->name + "\n" + res;
        cscope = cscope->parent_scope;
    }

    std::cout << res << std::endl;
    return; 
}

void AstEvaluator::new_var(std::string name, Value val) {
    current_scope->vars[name] = val;
}

void AstEvaluator::set_var(std::string name, Value val) {
    bool is_set = false;
    auto search_scope = current_scope;

    while(!is_set && search_scope) {
        auto it = search_scope->vars.find(name);
        if(it == search_scope->vars.end())
            search_scope = search_scope->parent_scope;
        else
            it->second = val, is_set = true;
    }
}

Value AstEvaluator::get_var(std::string name) {

    auto search_scope = current_scope;

    while(search_scope) {
        auto it = search_scope->vars.find(name);
        if(it == search_scope->vars.end())
            search_scope = search_scope->parent_scope;
        else {
            return it->second;
        }
    }

    return new_error(UnknownSym);
}


void AstEvaluator::new_func(AstNode node) {
    auto func = convert<_AstFunctionDefinition>(node);

    current_scope->funcs[func->name] = Func{0, MAXINT, [=] (Args a) {
        return call_func(func, a);
    }, false};
}

void AstEvaluator::new_func(std::string name, Func f) {
    current_scope->funcs[name] = Func{f.min_arg, f.max_arg, [=] (Args a) {
        create_new_scope(name + "_call"); 
        Value v = f.call(a);
        pop_scope();
        return v;
    }, f.eval_args_by_identifier};
}

Value AstEvaluator::call_func(AstFunctionDefinition node, Args a) {
    create_new_scope(node->name + std::string("()"));

    for(int i = 0; i < node->args.size() && i < a.size(); i++)
        new_var(node->args[i], a[i]);

    Value v = eval_block(node->action);
    v->return_value = false;
    pop_scope();

    return v;
}

Func AstEvaluator::get_func(std::string name) {
    bool is_found = false;

    auto search_scope = current_scope;

    while(!is_found && search_scope) {
        auto it = search_scope->funcs.find(name);
        if(it == search_scope->funcs.end())
            search_scope = search_scope->parent_scope;
        else
            return it->second;
    }

    return Func{0, 0, [] (Args a) {return new_error(UnknownSym);}};
}


/* end scope */


void AstEvaluator::add_builtin_functions() {
    
    new_func("sum", {2, MAXINT, [=] (Args a) {
        Value s = new_value(0);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s->long_val += x->long_val; 

        return s;
    }, false});

    new_func("product", {2, MAXINT, [=] (Args a) {
        Value s = new_value(1);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s->long_val *= x->long_val;

        return s;
    }, false});

    new_func("sum_range", {2, 2, [=] (Args a) {
        if(!check_args(a)) return new_error(BadValue);

        int start = a[0]->long_val;
        int end = a[1]->long_val;

        return new_value(end * (end + 1) / 2 - (start - 1) * (start) / 2);
    }, false});

    new_func("product_range", {2, 2, [=] (Args a) {
        Value v = new_value(1); 

        if(!check_args(a)) return new_error(BadValue);

        int start = a[0]->long_val;
        int end = a[1]->long_val;

        for(int i = start; i <= end; i++) {
            v->long_val *= i;
        }

        return v;
    }, false});

    new_func("print", {0, MAXINT, [=] (Args a) {
        for(int i = 0; i < a.size(); i++)
            std::cout << a[i] << " ";
        std::cout << std::endl;
        return new_error(NoValue); 
    }, false});

    new_func("trace", {0, 0, [=] (Args a) {print_trace(); return new_error(NoValue);}, false});


    // TODO: implement readln function
    new_func("readln", {1, MAXINT, [=] (Args a) {
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

    new_func("read", {1, MAXINT, [=] (Args a) {
            for(int i = 0; i < a.size(); i++) {
                
            }
            return new_error(UnknownSym); 
    }, true});

    new_func("import", {1, 1, [=] (Args a) {

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

Value AstEvaluator::eval_int(std::string str) {
    return new_value(std::atoi(str.c_str()));
}
//
//Args AstEvaluator::eval_args(AstNode node) {
//    Args args;
//
//    if(std::strstr(node->tag, "noarg")) {
//        return Args{};
//    }
//
//    if(!std::strstr(node->tag, "args"))
//        return Args{eval(node)};
//
//    else if(node->children_num == 0) {
//        AstNodeT as = new mpc_ast_t;
//        std::memcpy(as, node, sizeof(mpc_ast_t));
//
//        Value val = eval(as);
//        delete as;
//        return Args{val};
//    }
//
//
//    for(int i = 0; i < node->children_num; i += 2)
//        args.push_back(eval(node->children[i]));
//
//    return args;
//}
//
//Args AstEvaluator::eval_args_identifiers(AstNodeT node) {
//    Args args;
//
//    if(std::strstr(node->tag, "noarg"))
//        return Args{};
//
//    if(!std::strstr(node->tag, "args") || node->children_num == 0)
//        return Args{new_value(String(node->contents))};
//
//
//    for(int i = 0; i < node->children_num; i += 2)
//        args.push_back(new_value(String(node->children[i]->contents)));
//
//    return args;
//}

bool AstEvaluator::check_args(Args a) {
    for(auto arg : a)
        if(arg->type == ValueTypeError) return false;

    return true;
}


List AstEvaluator::eval_list(AstNode nnode) {
    auto node = convert<_AstList>(nnode);

    List lst;
    for(auto x : node->elems)
        lst.push_back(eval(x));

    return lst;
}

Value AstEvaluator::eval_bool(std::string str) {
    if(str == "false") return new_bvalue(false);
    else return new_bvalue(true);
}

Value AstEvaluator::eval_func(AstNode nnode) {
    auto node = convert<_AstFunctionCall>(nnode);
    auto fn = get_func(node->name);

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

Value AstEvaluator::eval_comp(AstNode nnode) {
    auto node = convert<_AstOperation> (nnode);
    return doit(eval(node->left), node->op, eval(node->right));
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


Value AstEvaluator::eval(AstNode node) {

    switch(node->tag) {

        case AstTagValue:
            return convert<_AstValue>(node)->value;

        case AstTagList:
            return new_value(eval_list(node));

        case AstTagVariable:
            return get_var(convert<_AstVariable>(node)->name);

        case AstTagOperation:
            return eval_comp(node);

        case AstTagFoldExpression: {
            auto expr = convert<_AstFoldExpr>(node);
            Value x = eval(expr->args[0]);

            for(int i = 1; i < expr->args.size(); i++)
                x = doit(x, expr->op, eval(expr->args[i])); 

            return x;
        }

        case AstTagVariableAssignment: {
            auto tmp = convert<_AstVariableAssignment>(node);
            Value val = eval(tmp->value);

            Value var = get_var(tmp->var_name);
            if(var->type == ValueTypeError && val->error == UnknownSym)
                return new_error(UnknownSym);

            set_var(tmp->var_name, val);
            return val;
        }

        case AstTagVariableDeclaration: {
            auto tmp = convert<_AstVariableDeclaration>(node);

            for(int i = 0; i < tmp->children.size(); i++) {

                switch(tmp->children[i]->tag) {
                    case AstTagVariableAssignment: {
                            auto var = convert<_AstVariableAssignment>(tmp->children[i]);
                            Value val = eval(var->value);
                            new_var(var->var_name, val);
                            break;
                    }

                    case AstTagVariable:
                        new_var(convert<_AstVariable>(tmp->children[i])->name, new_error(NoValue));
                        break;

                    default:
                        break;
                }
            }

            return new_error(NoValue);
        }

        case AstTagListQ: {
            auto listq = convert<_AstListQ>(node);                  

            Value arr = get_var(listq->name);

            for(auto index : listq->indices) {
                Value ind_val = eval(index);
                if(ind_val->type != ValueTypeNumber || arr->type != ValueTypeList)
                    return new_error(BadValue);

                auto size = arr->lst.size();
                auto ind = ind_val->long_val;

                if(ind < 0 || ind > size)
                    ind_val->long_val = (ind % size + size) % size;

                arr = (*arr)[ind_val];
            }

            return arr;
        } 

        case AstTagListAssignment: {
            auto lass = convert<_AstListAssignment>(node);
            auto left = eval(lass->left_side);
            *left = *eval(lass->right_side);
            return new_error(NoValue);
        }
    
        case AstTagConditional:
            return eval_if(node);
        
        case AstTagWhile:
            return eval_while(node);

        case AstTagFunctionDefinition : {
            new_func(node);
            return new_error(NoValue);
        }

        case AstTagFunctionCall:
            return eval_func(node);

        case AstTagReturn: {
            Value v = eval(convert<_AstReturn>(node)->expr);
            v->return_value = true;
            return v;
        }
        case AstTagBlock:
            return eval_block(convert<_AstBlock>(node), donotcreatescope, false);

        default:
            std::cout << "Hit eval(AstTagBlock/Expr). This means bug!" << std::endl;
    }

    return new_error(NoValue);
}

Value AstEvaluator::eval_block(AstBlock node, std::string new_scope, bool create_scope) {
    if(create_scope) create_new_scope(new_scope);

    for(int i = 0; i < node->children.size(); i++) {
        Value v = eval(node->children[i]);
        if(v->return_value) {
            if(create_scope) pop_scope();
            return v;
        }
    }

    if(create_scope) pop_scope();
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

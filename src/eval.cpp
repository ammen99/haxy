#include "eval.hpp"

void Evaluator::init(mpc_parser_t *p) {
    this->parser = p;

    create_new_scope("__global__");
    add_builtin_functions();
}

/* definitions for Scope */

void Evaluator::create_new_scope(std::string name) {
    Scope *new_scope = new Scope;
    new_scope->name = name;
    new_scope->parent_scope = current_scope;

    current_scope = new_scope;
}

void Evaluator::pop_scope() {
    if(!current_scope) return;
    auto parent_scope = current_scope->parent_scope;

    delete current_scope;
    current_scope = parent_scope;
}

void Evaluator::new_var(std::string name, Value val) {
    std::cout << "new var " << name << " " << val << std::endl;
    current_scope->vars[name] = val;
    std::cout << "gut" << std::endl;
}

void Evaluator::set_var(std::string name, Value val) {
    bool is_set = false;

    auto search_scope = current_scope;

    while(!is_set && search_scope) {
        std::cout << "searching for " << name << " in " << search_scope->name<< std::endl;
        std::cout << search_scope << std::endl; 
        auto it = search_scope->vars.find(name);
        std::cout << "hello" << std::endl;
        if(it == search_scope->vars.end())
            search_scope = search_scope->parent_scope;
        else
            std::cout << "found" << std::endl, it->second = val, is_set = true;
    }
}

Value Evaluator::get_var(std::string name) {

    auto search_scope = current_scope;

    while(search_scope) {
        auto it = search_scope->vars.find(name);
        if(it == search_scope->vars.end())
            search_scope = search_scope->parent_scope;
        else
            return it->second;
    }

    return new_error(UnknownSym);
}


void Evaluator::new_func(std::string name, AstNode node) {
    std::cout << "Creating a new function with name = " << name << std::endl;
    current_scope->funcs[name] = Func{0, MAXINT, [=] (Args a) {
        return call_func(node, a);
    }};
}

void Evaluator::new_func(std::string name, Func f) {
    current_scope->funcs[name] = f;
}

Value Evaluator::call_func(AstNode node, Args a) {
    create_new_scope(node->children[0]->contents + std::string("_call"));

    std::cout << "Call_func " << a.size() << std::endl;

    std::cout << node->children[1]->tag << std::endl;
    auto args = node->children[1]->children[2];    
    std::cout << "found node " << args->tag << std::endl;

    for(int i = 0; i < args->children_num && i < a.size() * 2; i += 2) {
        std::cout << "create var" << std::endl;;
        new_var(args->children[i]->contents, a[i / 2]); 
        std::cout << "created var" << std::endl;
    }

    std::cout << "eval block" << std::endl;
    eval_block(node->children[2]);

    pop_scope();

    return new_error(NoValue);
}

Func Evaluator::get_func(std::string name) {
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


void Evaluator::add_builtin_functions() {
    
    new_func("sum", {2, MAXINT, [=] (Args a) {
        Value s = new_value(0);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s.long_val += x.long_val; 

        return s;
    }, false});

    new_func("product", {2, MAXINT, [=] (Args a) {
        Value s = new_value(1);

        if(!check_args(a)) return new_error(BadValue);

        for(auto x : a)
            s.long_val *= x.long_val;

        return s;
    }, false});

    new_func("sum_range", {2, 2, [=] (Args a) {
        if(!check_args(a)) return new_error(BadValue);

        int start = a[0].long_val;
        int end = a[1].long_val;

        return new_value(end * (end + 1) / 2 - (start - 1) * (start) / 2);
    }, false});

    new_func("product_range", {2, 2, [=] (Args a) {
        Value v = new_value(1); 

        if(!check_args(a)) return new_error(BadValue);

        int start = a[0].long_val;
        int end = a[1].long_val;

        for(int i = start; i <= end; i++) {
            v.long_val *= i;
        }

        return v;
    }, false});

    new_func("print", {0, MAXINT, [=] (Args a) {
        for(int i = 0; i < a.size(); i++)
            std::cout << a[i] << "| ";
        std::cout << std::endl;
        return new_error(NoValue); 
    }, false});

    new_func("read", {1, MAXINT, [=] (Args a) {
        for(int i = 0; i < a.size(); i++) {
            switch(a[i].type) {
                case ValueTypeBool:
                case ValueTypeNumber:
                    std::cin >> a[i].long_val;
                    break;
                case ValueTypeDbl:
                    std::cin >> a[i].dbl;
                    break;
                case ValueTypeList:
                    return new_error(BadOp);
                    break;
                case ValueTypeString:
                    std::getline(std::cin, a[i].str);
                    break;
                case ValueTypeError:
                    break;
            }     
        }
        return new_error(NoValue);        
    }, true});

    new_func("import", {1, 1, [=] (Args a) {

        if(a[0].type != ValueTypeString)
            return new_error(BadValue);

        eval_file(a[0].str + ".hx");
        return new_error(NoValue); 
    }, true});
}

/* different functions for evaluating */

/* does and operation expression */
Value doit(Value a, char* op, Value b) {
    std::string o = op;

    if(o == "+") return a + b;
    if(o == "-") return a - b;
    if(o == "*") return a * b;
    if(o == "/") return a / b;
    if(o == "||") return a || b;
    if(o == "&&") return a && b;

    if(o == "==") return a == b;
    if(o == ">=") return a >= b;
    if(o == "<=") return a <= b;
    if(o == "!=") return a != b;
    if(o == ">")  return a > b;
    if(o == "<")  return a < b;

    return new_error(BadOp);
}

Value Evaluator::eval_int(std::string str) {
    long val = 0;
    for(int i = 0; i < str.length(); i++) {
        if(!std::isdigit(str[i])) return new_error(BadValue); 
        val *= 10;
        val += str[i] - '0';
    }

    return new_value(val);
}

Args Evaluator::eval_args(AstNode node) {
    std::cout << "Eval args" << std::endl;

    Args args;

    if(std::strstr(node->tag, "noarg")) {
        return Args{};
    }

    if(!std::strstr(node->tag, "args")) {
        std::cout << "single args with expr" << std::endl;
        return Args{eval(node)};
    }
    else if(node->children_num == 0) {
        std::cout << "No args given" << std::endl;
        AstNode as = new mpc_ast_t;
        std::memcpy(as, node, sizeof(mpc_ast_t));

        Value val = eval(as);
        delete as;
        return Args{val};
    }


    std::cout << "Args num children " << node->children_num << std::endl;
    for(int i = 0; i < node->children_num; i += 2) {
        args.push_back(eval(node->children[i]));
    }

    std::cout << "num args final" << args.size() << std::endl;
    return args;
}

Args Evaluator::eval_args_identifiers(AstNode node) {
    Args args;

    if(std::strstr(node->tag, "noarg"))
        return Args{};

    if(!std::strstr(node->tag, "args") || node->children_num == 0)
        return Args{new_value(String(node->contents))};


    for(int i = 0; i < node->children_num; i += 2)
        args.push_back(new_value(String(node->children[i]->contents)));

    return args;
}

bool Evaluator::check_args(Args a) {
    for(auto arg : a)
        if(arg.type == ValueTypeError) return false;

    return true;
}


List Evaluator::eval_list(AstNode node) {
    Args a = eval_args(node->children[1]); 

    List lst;
    for(auto x : a)
        lst.push_back(x);

    std::cout << lst.size() << std::endl;
    return lst;
}

Value Evaluator::eval_bool(std::string str) {
    if(str == "false") return new_bvalue(false);
    else return new_bvalue(true);
}

Value Evaluator::eval_func(AstNode node) {
    std::string name = node->children[0]->contents;
    auto fn = get_func(name);

    Args args;

    if(fn.eval_args_by_identifier)
        args = eval_args_identifiers(node->children[2]);
    else
        args = eval_args(node->children[2]);

    std::cout << "Execute func " << name << " with args.size() = " << args.size() << std::endl;

    if(fn.min_arg <= args.size() && args.size() <= fn.max_arg)
        return fn.call(args);
    else
        return new_error(ArgumentNumberMismatch);
    
}

Value Evaluator::eval_comp(AstNode node) {
    Value a = eval(node->children[0]);
    char *op = node->children[1]->contents;
    Value b = eval(node->children[2]);

    std::cout << "Eval doit " << std::endl;
    return doit(a, op, b);
}

bool Evaluator::eval_if(AstNode node) {

    Value res = eval(node->children[1]);
    if((res.type & ValueTypeBoolArithm) && res.long_val) eval_block(node->children[2]);
    else if (res.type == ValueTypeError) std::cout << res << std::endl;
    else return false;

    return true;
}

bool Evaluator::eval_elif(AstNode node) {
    if(std::strstr(node->children[0]->tag, "elif")) {
        if(eval_elif(node->children[0])) return true; 
    }
    else if(std::strstr(node->children[0]->tag, "if")) {
        if(eval_if(node->children[0])) return true;
    }

    Value v = eval(node->children[2]);
    if(v.type != ValueTypeError && v.long_val) eval_block(node->children[3]);
    else if(v.type == ValueTypeError) std::cout << v << std::endl;
    else return false;

    return true;
}

void Evaluator::eval_else(AstNode node) {
    if(std::strstr(node->children[0]->tag, "elif")) {
        if(eval_elif(node->children[0])) return;
    }
    
    if(std::strstr(node->children[0]->tag, "if")) {
        if(eval_if(node->children[0])) return;
    }

    eval_block(node->children[2]);
}

void Evaluator::eval_while(AstNode node) {
    auto val = eval(node->children[1]);

    while((val.type & ValueTypeBoolArithm) && val.long_val) {
        eval_block(node->children[2]);
        val = eval(node->children[1]);
    }
}


Value Evaluator::eval(AstNode node) {

    if(std::strstr(node->tag, "number"))
        return eval_int(node->contents);

    else if(std::strstr(node->tag, "dbl"))
        return new_dvalue(std::stod(node->contents));
    
    else if(std::strstr(node->tag, "bool"))
        return eval_bool(node->contents); 

    else if(std::strstr(node->tag, "str")) {
        std::cout << "strstr" << std::endl;
        auto tmp = String(node->contents);
        return new_value(tmp.substr(1, tmp.length() - 2));
    }

    else if(std::strstr(node->tag, "var")) {
        if(node->children_num == 2) { // initialize variable
            std::string name = node->children[1]->children[0]->contents;
            Value val = eval(node->children[1]->children[2]);
            new_var(name, val);

            return val;
        }
        
        else {
            std::string name = node->children[2]->contents;
            new_var(name, new_error(NoValue));
            return new_error(NoValue);
        }

    }

    else if(std::strstr(node->tag, "assign")) {
        std::string name = node->children[0]->contents;
        Value val = eval(node->children[2]);
        set_var(name, val);
        return val;
    }

    else if(std::strstr(node->tag, "else")) {
        eval_else(node);
        return new_error(NoValue);
    }
    else if(std::strstr(node->tag, "elif")) {
        eval_elif(node);
        return new_error(NoValue);
    }
    else if(std::strstr(node->tag, "if")) {
        eval_if(node);
        return new_error(NoValue);
    }

    else if(std::strstr(node->tag, "while")) {
        eval_while(node);
        return new_error(NoValue);
    }

    else if(std::strstr(node->tag, "ident")) {
        std::string name = node->contents;
        return get_var(name);
    }

    else if(std::strstr(node->tag, "gcomp"))
        return eval_comp(node->children[1]);
    else if(std::strstr(node->tag, "comp"))
        return eval_comp(node);

    else if(std::strstr(node->tag, "expr")) {

        std::cout << "Expression " << std::endl;
        char *op = node->children[1]->contents;
        Value x = eval(node->children[2]);

        int i = 3;
        while(is_computable(node->children[i]->tag)) {
            x = doit(x, op, eval(node->children[i]));
            ++i;
        }

        return x;
    }

    else if(std::strstr(node->tag, "if")) {
        eval_if(node);
        return new_error(NoValue);
    }

    else if(std::strstr(node->tag, "fundef")){
        new_func(node->children[1]->children[0]->contents, node);
        return new_error(NoValue);
    }

    else if(std::strstr(node->tag, "func"))
        return eval_func(node);

    else if(std::strstr(node->tag, "list"))
        return new_value(eval_list(node));

    else {
        for(int i = 0; i < node->children_num; i++) {
            auto v = eval(node->children[i]);
            if(v.type != ValueTypeError) return v;
            else if(v.error != NoValue) return v;
        }
    }

    return new_error(NoValue);
}

void Evaluator::eval_block(AstNode node) {
    for(int i = 1; i < node->children_num - 1; i++)
        std::cout << "Eval block " << eval(node->children[i]) << std::endl;
}

void Evaluator::eval_file(std::string name) {
    std::string src = load_file(name);

    mpc_result_t res;

    if(mpc_parse("input", src.c_str(), parser, &res)) {
        mpc_ast_print(ast(res.output)); 
        eval_block(ast(res.output));
        loaded_asts.push_back(ast(res.output));
    }
    else {
        mpc_err_print(res.error),
        mpc_err_delete(res.error);
    }
}

void Evaluator::cleanup() {
    for(auto x : loaded_asts)
        mpc_ast_delete(x);
}
/* end eval_xxx */

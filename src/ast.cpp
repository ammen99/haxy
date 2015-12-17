#include "ast.hpp"
#include <sstream>

#define new_node(name, type) type name = ptr::shared_ptr<_ ## type>();
#define new_astvalue(name) AstValue name = ptr::shared_ptr<_AstValue>(); \
                           name->tag = AstTagValue; \


namespace haxy {

    AstNode AstGenerator::generate (AstNodeT root) {

        if(std::strstr(root->tag, "number")) {
            new_astvalue(node);
            int x = std::atoi(root->contents);
            node->value = new_value(x);
            return node;
        }

        else if(std::strstr(root->tag, "dbl")) {
            new_astvalue(node);
            double d = std::stod(root->contents);
            node->value = new_value(d);
            return node;
        }

        else if(std::strstr(root->tag, "bool")) {
            new_astvalue(node);
            bool b = (std::string(root->contents) == "false");
            node->value = new_bvalue(b);
            return node;
        }

        else if(std::strstr(root->tag, "str")) {
            new_astvalue(node);
            auto tmp = String(root->contents);
            node->value = new_value(tmp.substr(1, tmp.length() - 2));
            return node;
        }

        else if(std::strstr(root->tag, "return")) {
            new_node(node, AstReturn);
            node->tag = AstTagReturn;
            node->expr = generate(root->children[1]);
            return node;
        }

        else if(std::strstr(root->tag, "listq")) {
            new_node(node, AstListQ);

            node->tag = AstTagListQ;
            node->name = root->children[0]->contents;

            for(int i = 2; i < root->children_num; i+= 3) 
                node->indices.push_back(generate(root->children[i]));

            return node;
        }

        else if(std::strstr(root->tag, "var"))
            return generate_vardecl(root);

        else if(std::strstr(root->tag, "assign")) {
            new_node(node, AstAssignment);
            node->tag = AstTagAssignment;

            node->left_side = generate(root->children[0]);
            node->right_side = generate(root->children[2]);

            return node;
        }

        else if(std::strstr(root->tag, "cond"))
            return generate_if(root);

        else if(std::strstr(root->tag, "if")) {
            new_node(node, AstConditional);
            node->tag = AstTagConditional;

            new_node(if_node, AstIf);
            if_node->type = IfTypeIf;
            if_node->expr = generate(root->children[1]);
            if_node->block = generate_block(root->children[2]);

            node->children.push_back(if_node);
            return node;
        }

        else if(std::strstr(root->tag, "while"))
            return generate_while(root);

        else if(std::strstr(root->tag, "ident")) {
            new_node(node, AstVariable);
            node->tag = AstTagVariable;
            node->name = root->contents;

            return node;
        }

        else if(std::strstr(root->tag, "gcomp"))
            return generate_comp(root->children[1]);
        else if(std::strstr(root->tag, "comp"))
            return generate_comp(root);

        else if(std::strstr(root->tag, "expr")) {
            new_node(node, AstOperation);
            node->tag = AstTagOperation;

            for(int i = 2; i < root->children_num - 1; i++) {
                node->args.push_back((generate(root->children[i]))); 
                node->ops.push_back(root->children[1]->contents);
            }

            return node;
        }

        else if(std::strstr(root->tag, "fundef"))
            return generate_func_def(root);

        else if(std::strstr(root->tag, "func"))
            return generate_func_call(root);

        else if(std::strstr(root->tag, "list"))
            return generate_list(root);

        else if(std::strstr(root->tag, "state"))
            return generate(root->children[0]);

        else if(std::strstr(root->tag, "class")) {
            new_node(node, AstClass);
            node->tag = AstTagClass;
            node->name = root->children[1]->contents;

            /* children are either variable declarations or function definitions */
            for(int i = 3; i < root->children_num - 1; i++) {
                if(std::strstr(root->children[i]->tag, "var"))
                    node->vars.push_back(generate_vardecl(root->children[i]));
                else
                    node->funcs.push_back(generate_func_def(root->children[i]));
            }

            return node;
        }

        else if(std::strstr(root->tag, "member")) {
            new_node(node, AstClassReference);
            node->tag = AstTagClassRef;

            for(int i = 0; i < root->children_num; i += 2)
                node->refs.insert(node->refs.begin(), generate(root->children[i])); 

            return node;
        }

        new_node(node, AstValue);
        node->tag = AstTagValue;
        node->value = new_error(NoValue);

        return node;
    }

    AstBlock AstGenerator::generate_block(AstNodeT root) {
        new_node(node, AstBlock);
        node->tag = AstTagBlock;

        for(int i = 1; i < root->children_num - 1; i++)
            node->children.push_back(generate(root->children[i]));

        return node;
    }

    AstNode AstGenerator::generate_comp(AstNodeT root) {
        new_node(node, AstOperation);
        node->tag = AstTagOperation;

        for(int i = 0; i < root->children_num; i += 2) {
            node->args.push_back(generate(root->children[i])); 
            if(i != root->children_num - 1)
                node->ops.push_back(root->children[i + 1]->contents);
        }

        return node;
    }

    std::vector<AstNode> AstGenerator::parse_args(AstNodeT root) {
        std::vector<AstNode> v; 

        if(std::strstr(root->tag, "noarg")) {}
        else if(!std::strstr(root->tag, "args") || root->children_num == 0)
            v = {generate(root)};
        else 
            for(int i = 0; i < root->children_num; i += 2)
                v.push_back(generate(root->children[i]));
        return v;

    }

    AstNode AstGenerator::generate_list(AstNodeT root) {
        new_node(node, AstList);
        node->tag = AstTagList;

        node->elems = parse_args(root->children[1]);

        return node;
    }


    AstVariableDeclaration AstGenerator::generate_vardecl(AstNodeT root) {
        new_node(node, AstVariableDeclaration);
        node->tag = AstTagVariableDeclaration;

        for(int i = 1; i < root->children_num; i += 2) {

            if(std::strstr(root->children[i]->tag, "assign")) { // initialize variable

                new_node(child, AstAssignment);
                child->tag = AstTagAssignment;

                child->left_side = generate(root->children[i]->children[0]);
                child->right_side = generate(root->children[i]->children[2]);

                node->children.push_back(child);
            }

            else {
                new_node(child, AstVariable);
                child->tag = AstTagVariable;
                child->name = root->children[i]->contents;

                node->children.push_back(child);
            }
        }

        return node;

    }

    AstNode AstGenerator::generate_if(AstNodeT root) {
        new_node(node, AstConditional);
        node->tag = AstTagConditional;

        for(int i = 0; i < root->children_num; i++) {
            new_node(child, AstIf);
            if(std::strstr(root->children[i]->tag, "else")) {
                child->type = IfTypeElse;
                child->block = generate_block(root->children[i]->children[1]);
            } 

            else if(std::strstr(root->children[i]->tag, "elif")) {
                child->type = IfTypeElif;
                child->expr = generate(root->children[i]->children[1]);
                child->block = generate_block(root->children[i]->children[2]);
            }
            else {
                child->type = IfTypeIf;
                child->expr = generate(root->children[i]->children[1]);
                child->block = generate_block(root->children[i]->children[2]);
            }

            node->children.push_back(child);
        }

        return node;
    }

    AstNode AstGenerator::generate_while(AstNodeT root) {
        new_node(node, AstWhile);
        node->tag = AstTagWhile;

        node->condition = generate(root->children[1]);
        node->action = generate_block(root->children[2]);

        return node;
    }

    AstFunctionDefinition AstGenerator::generate_func_def(AstNodeT root) {
        new_node(node, AstFunctionDefinition);
        node->tag = AstTagFunctionDefinition;
        node->name = root->children[1]->children[0]->contents;

        auto args_root = root->children[1]->children[2];
        if(std::strstr(args_root->tag, "noarg")) {}
        else if(args_root->children_num == 0) node->args.push_back(args_root->contents);
        else 
            for(int i = 0; i < args_root->children_num; i += 2)
                node->args.push_back(args_root->children[i]->contents);

        node->action = generate_block(root->children[2]);
        return node;
    }

    AstNode AstGenerator::generate_func_call(AstNodeT root) {
        new_node(node, AstFunctionCall);
        node->tag = AstTagFunctionCall;

        node->name = root->children[0]->contents;
        node->args = parse_args(root->children[2]);

        return node;
    }

    AstNode AstGenerator::parse_file(AstNodeT root) {
        return generate_block(root);
    }

#define out (to_stdout ? std::cout : stream)
   
    void AstPrinter::write(std::string str, int depth) {
        for(int i = 0; i < depth; i++) out << " ";
        out << str;
    }

    void AstPrinter::write(Value v, int depth) {
        for(int i = 0; i < depth; i++) out << " ";
        out << v;
    }

    void AstPrinter::write(AstNode node, int depth) {
        switch(node->tag) {
            case AstTagValue:
                write("value = ", depth);
                write(node.convert<_AstValue>()->value, 0);
                write("\n", 0);
                break;
            case AstTagVariable:
                write("var(name = ", depth);
                write(node.convert<_AstVariable>()->name, 0);
                write(")\n", 0);
                break;

            case AstTagList: {
                write("list:\n", depth);
                auto list = node.convert<_AstList>();
                for(int i = 0; i < list->elems.size(); i++) {
                    write(list->elems[i], depth + 4);
                };
                break;
            }

            case AstTagListQ: {
                auto listq = node.convert<_AstListQ>();
                write("listq:\n", depth);
                write("name = ", depth + 4);
                write(listq->name + "\n", 0);
                write("indices:\n", depth + 4);
                for(int i = 0; i < listq->indices.size(); i++)
                    write(listq->indices[i], depth + 8);

                break;
            }

            case AstTagWhile: {
                auto wh = node.convert<_AstWhile>();
                write("while:\n", depth);
                write("condition:\n", depth + 4);
                write(wh->condition, depth + 8);
                write(wh->action, depth + 4);
                break;
            }

            case AstTagBlock: {
                auto block = node.convert<_AstBlock>();
                write("block:\n", depth);
                for(int i = 0; i < block->children.size(); i++)
                    write(block->children[i], depth + 4);
            
                break;
            }

            case AstTagReturn: {
                write("return: \n", depth);
                write(node.convert<_AstReturn>()->expr, depth + 4);
                break;
            }

            case AstTagFunctionDefinition: {
                write("newfunc:\n", depth);                               
                write("name = ", depth + 4);

                auto fun = node.convert<_AstFunctionDefinition>();
                write(fun->name);
                out << "\n";
                write("args = (", depth + 4);
                for(int i = 0; i < fun->args.size(); i++) {
                    write(fun->args[i], 0);
                    if(i != fun->args.size() - 1) write(" ", 0);
                }

                out << ")\n";
                
                write(fun->action, depth + 4);
                break;
            }

            case AstTagAssignment: {
                write("assignment:\n", depth);                               
                write("left_side = \n", depth + 4);
                
                auto assign = node.convert<_AstAssignment>();
                write(assign->left_side, depth + 8);

                write("left_side = \n", depth + 4);
                write(assign->right_side, depth + 8);
                break;
            }

            case AstTagVariableDeclaration: {
                write("vardecl:\n", depth);

                auto decl = node.convert<_AstVariableDeclaration>();
                for(int i = 0; i < decl->children.size(); i++) {
                    write(decl->children[i], depth + 4);
                }
                break;
            }

            case AstTagOperation: {
                write("operation:\n", depth);                           
                auto expr = node.convert<_AstOperation>();

                for(int i = 0; i < expr->args.size(); i++) {
                    write(expr->args[i], depth + 4);
                    if(i != expr->args.size() - 1)
                        write(expr->ops[i] + "\n", depth + 4);
                }
                
                break;
            }

            case AstTagFunctionCall: {
                auto funcall = node.convert<_AstFunctionCall>();

                write("call " + funcall->name + ":\n", depth);
                for(int i = 0; i < funcall->args.size(); i++)
                    write(funcall->args[i], depth + 4);
                break;                         
            }

            case AstTagConditional: {
                auto cond = node.convert<_AstConditional>();                        
                write("conditional:\n", depth);

                for(int i = 0; i < cond->children.size(); i++) {
                    if(cond->children[i]->type == IfTypeIf) {
                        write("if:\n", depth + 4);
                        write(cond->children[i]->expr, depth + 8);
                        write(cond->children[i]->block, depth + 8);
                    }

                    if(cond->children[i]->type == IfTypeElif) {
                        write("elif:\n", depth + 4);
                        write(cond->children[i]->expr, depth + 8);
                        write(cond->children[i]->block, depth + 8);                   
                    }

                    if(cond->children[i]->type == IfTypeElse) {
                        write("else:\n", depth + 4);
                        write(cond->children[i]->block, depth + 8);
                    }
                }
                out << "\n";
                break;
            }

            case AstTagClass: {
                auto cl = node.convert<_AstClass>();

                write("class:\n", depth);  
                write("name = " + cl->name + "\n", depth + 4);
                for(auto x : cl->vars)
                    write(x, depth + 4);
                for(auto x : cl->funcs)
                    write(x, depth + 4);

                break;
            }

            case AstTagClassRef: {
                auto clref = node.convert<_AstClassReference>();                     

                write("class ref:\n", depth);
                for(auto x : clref->refs)
                    write(x, depth + 4);

                break;
            }

            case AstTagExpr: {
                break;
            }
        }
    }

    void AstPrinter::write_tree(AstNode n) {
        write(n, 0);
    }
    /* end AstGenerator */
}


#include "ast.hpp"
#include <sstream>

#define new_node(name) AstNode name = std::make_shared<_AstNode>();
#define new_astvalue(name) AstValue name = std::make_shared<_AstValue>(); \
                           name->tag = AstTagValue; \


namespace haxy {
    template<class T> 
    inline std::shared_ptr<T> convert(AstNode node) {
        return std::static_pointer_cast<T>(node);
    }

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
            auto node = std::make_shared<_AstReturn>();
            node->tag = AstTagReturn;
            node->expr = generate(root->children[1]);
            return node;
        }

        else if(std::strstr(root->tag, "modlist")) {
            auto node = std::make_shared<_AstListAssignment>();
            node->tag = AstTagListAssignment;
            node->left_side = convert<_AstListQ>(generate(root->children[0]));
            node->right_side = generate(root->children[2]);

            return node;
        }

        else if(std::strstr(root->tag, "listq")) {
            auto node = std::make_shared<_AstListQ>();

            node->tag = AstTagListQ;
            node->name = root->children[0]->contents;

            for(int i = 2; i < root->children_num; i+= 3) 
                node->indices.push_back(generate(root->children[i]));

            return node;
        }

        else if(std::strstr(root->tag, "var"))
            return generate_vardecl(root);

        else if(std::strstr(root->tag, "assign")) {
            auto node = std::make_shared<_AstVariableAssignment>();
            node->tag = AstTagVariableAssignment;

            node->var_name = root->children[0]->contents;
            node->value = generate(root->children[2]);

            return node;
        }

        else if(std::strstr(root->tag, "cond"))
            return generate_if(root);

        else if(std::strstr(root->tag, "if")) {
            auto node = std::make_shared<_AstConditional>();
            node->tag = AstTagConditional;

            auto if_node = std::make_shared<_AstIf>();
            if_node->type = IfTypeIf;
            if_node->expr = convert<_AstNode>(generate(root->children[1]));
            if_node->block = generate_block(root->children[2]);

            node->children.push_back(if_node);

            return node;
        }

        else if(std::strstr(root->tag, "while"))
            return generate_while(root);

        else if(std::strstr(root->tag, "ident")) {
            auto node = std::make_shared<_AstVariable>();
            node->tag = AstTagVariable;
            node->name = root->contents;

            return node;
        }

        else if(std::strstr(root->tag, "gcomp"))
            return generate_comp(root->children[1]);
        else if(std::strstr(root->tag, "comp"))
            return generate_comp(root);

        else if(std::strstr(root->tag, "expr")) {
            auto node = std::make_shared<_AstFoldExpr>();
            node->tag = AstTagFoldExpression;
            node->op = root->children[1]->contents;

            for(int i = 2; i < root->children_num - 1; i++) {
                node->args.push_back(convert<_AstFoldExpr>(generate(root->children[i]))); 
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
            auto node = std::make_shared<_AstClass>(); 
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

        auto node = std::make_shared<_AstValue>();
        node->tag = AstTagValue;
        node->value = new_error(NoValue);

        return node;
    }

    AstBlock AstGenerator::generate_block(AstNodeT root) {
        auto node = std::make_shared<_AstBlock>();
        node->tag = AstTagBlock;

        for(int i = 1; i < root->children_num - 1; i++)
            node->children.push_back(generate(root->children[i]));

        return node;
    }

    AstNode AstGenerator::generate_comp(AstNodeT root) {
        auto node = std::make_shared<_AstOperation>(); 
        node->tag = AstTagOperation;

        node->left = generate(root->children[0]);
        node->right = generate(root->children[2]);
        
        node->op = root->children[1]->contents;

        return node;
    }

    std::vector<AstNode> AstGenerator::parse_args(AstNodeT root) {
        std::vector<AstNode> v; 

        if(std::strstr(root->tag, "noarg")) {}
        else if(!std::strstr(root->tag, "args") || root->children_num == 0)
            v = {convert<_AstExpr>(generate(root))};
        else 
            for(int i = 0; i < root->children_num; i += 2)
                v.push_back(convert<_AstExpr>(generate(root->children[i])));
        return v;

    }

    AstNode AstGenerator::generate_list(AstNodeT root) {
        auto node = std::make_shared<_AstList>();
        node->tag = AstTagList;

        node->elems = parse_args(root->children[1]);

        return node;
    }


    AstVariableDeclaration AstGenerator::generate_vardecl(AstNodeT root) {
        auto node = std::make_shared<_AstVariableDeclaration>();
        node->tag = AstTagVariableDeclaration;

        for(int i = 1; i < root->children_num; i += 2) {

            if(std::strstr(root->children[i]->tag, "assign")) { // initialize variable
                auto child = std::make_shared<_AstVariableAssignment>();

                child->tag = AstTagVariableAssignment;
                child->var_name = root->children[i]->children[0]->contents;
                child->value = generate(root->children[i]->children[2]);

                node->children.push_back(child);
            }

            else {
                auto child = std::make_shared<_AstVariable>();
                child->tag = AstTagVariable;
                child->name = root->children[i]->contents;

                node->children.push_back(child);
            }
        }

        return node;

    }

    AstNode AstGenerator::generate_if(AstNodeT root) {
        auto node = std::make_shared<_AstConditional>();         
        node->tag = AstTagConditional;

        for(int i = 0; i < root->children_num; i++) {
            auto child = std::make_shared<_AstIf>();
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
        auto node = std::make_shared<_AstWhile>(); 
        node->tag = AstTagWhile;

        node->condition = generate(root->children[1]);
        node->action = generate_block(root->children[2]);

        return node;
    }

    AstFunctionDefinition AstGenerator::generate_func_def(AstNodeT root) {
        auto node = std::make_shared<_AstFunctionDefinition> ();
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
        auto node = std::make_shared<_AstFunctionCall>(); 
        node->tag = AstTagFunctionCall;

        node->name = root->children[0]->contents;
        node->args = parse_args(root->children[2]);

        return node;
    }

    AstNode AstGenerator::parse_file(AstNodeT root) {
        return generate_block(root);
    }


    /* end AstGenerator */

    /* begin AstWriter */

#define out (to_stdout ? std::cout : stream)
   
    void AstWriter::write(std::string str, int depth) {
        for(int i = 0; i < depth; i++) out << " ";
        out << str;
    }

    void AstWriter::write(Value v, int depth) {
        for(int i = 0; i < depth; i++) out << " ";
        out << v;
    }

    void AstWriter::write(AstNode node, int depth) {
        switch(node->tag) {
            case AstTagValue:
                write("value = ", depth);
                write(convert<_AstValue>(node)->value, 0);
                write("\n", 0);
                break;
            case AstTagVariable:
                write("var(name = ", depth);
                write(convert<_AstVariable>(node)->name, 0);
                write(")\n", 0);
                break;

            case AstTagList: {
                write("list:\n", depth);
                auto list = convert<_AstList>(node);
                for(int i = 0; i < list->elems.size(); i++) {
                    write(list->elems[i], depth + 4);
                };
                break;
            }

            case AstTagListQ: {
                auto listq = convert<_AstListQ>(node);
                write("listq:\n", depth);
                write("name = ", depth + 4);
                write(listq->name, 0);
                out << "\n";
                write("index = ", depth + 4);
                for(int i = 0; i < listq->indices.size(); i++)
                    write(listq->indices[i], 0),
                    out << " ";

                break;
            }

            case AstTagWhile: {
                auto wh = convert<_AstWhile>(node);
                write("while:\n", depth);
                write("condition:\n", depth + 4);
                write(wh->condition, depth + 8);
                write(wh->action, depth + 4);
                break;
            }

            case AstTagBlock: {
                auto block = convert<_AstBlock>(node);
                write("block:\n", depth);
                for(int i = 0; i < block->children.size(); i++)
                    write(block->children[i], depth + 4);
            
                break;
            }

            case AstTagReturn: {
                write("return: \n", depth);
                write("value = ", depth + 4);
                write(convert<_AstReturn>(node)->expr, 0);
                break;
            }

            case AstTagFunctionDefinition: {
                write("newfunc:\n", depth);                               
                write("name = ", depth + 4);

                auto fun = convert<_AstFunctionDefinition>(node);
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

            case AstTagVariableAssignment: {
                write("assignment:\n", depth);                               
                write("name = ", depth + 4);
                
                auto assign = convert<_AstVariableAssignment>(node);
                write(assign->var_name + "\n", 0);
                write(assign->value, depth + 4);
                break;
            }

            case AstTagVariableDeclaration: {
                write("vardecl:\n", depth);

                auto decl = convert<_AstVariableDeclaration>(node);
                for(int i = 0; i < decl->children.size(); i++) {
                    write(decl->children[i], depth + 4);
                }
                break;
            }

            case AstTagListAssignment: {
                write("listassign:\n", depth);
                
                auto list = convert<_AstListAssignment>(node);
                write(list->left_side, depth + 4);
                write(list->right_side, depth + 4);
                break;
            }

            case AstTagFoldExpression: {
                write("foldexpr:\n", depth);                           
                write("op = ", depth + 4);

                auto expr = convert<_AstFoldExpr>(node);

                write(expr->op, 0);
                out << "\n";

                for(int i = 0; i < expr->args.size(); i++)
                    write(expr->args[i], depth + 4);
                
                break;
            }

            case AstTagFunctionCall: {
                auto funcall = convert<_AstFunctionCall>(node);

                write("call " + funcall->name + ":\n", depth);
                for(int i = 0; i < funcall->args.size(); i++)
                    write(funcall->args[i], depth + 4);
                break;                         
            }

            case AstTagConditional: {
                auto cond = convert<_AstConditional>(node);                        
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

            case AstTagOperation: {
                auto op = convert<_AstOperation>(node);
                write("operation: ", depth);
                write(op->op + "\n", 0);
                write(op->left, depth + 4);
                write(op->right, depth + 4);
                break;                      
            }

            case AstTagClass: {
                auto cl = convert<_AstClass>(node);

                write("class:\n", depth);  
                write("name = " + cl->name + "\n", depth + 4);
                for(auto x : cl->vars)
                    write(x, depth + 4);
                for(auto x : cl->funcs)
                    write(x, depth + 4);

                break;
            }

            case AstTagExpr: {
                break;
            }
        }
    }

    void AstWriter::write_tree(AstNode n) {
        write(n, 0);
    }

    /* begin AstWriter */

#undef out
#define out (!write_mainstream ? topstream : mainstream)
//#define out (std::cout)
   
    void AstCppTranslator::write(std::string str, int depth) {
        for(int i = 0; i < depth; i++) out << " ";
        out << str;
    }

    void AstCppTranslator::write(Value v, int depth) {
        for(int i = 0; i < depth; i++) out << " ";
        switch(v->type) {
            case ValueTypeError:
                std::cerr << "Error while translating, exiting" << std::endl;
                std::exit(-1);
                break;

            case ValueTypeNumber:
                out << "new_value(" << v->long_val << ")";
                break;

            case ValueTypeDbl:
                out << "new_dvalue(" << v->dbl << ")";
                break;

            case ValueTypeBool:
                out << "new_bvalue(" << v->long_val << ")";
                break;

            case ValueTypeString:
                out << "new_value(String{\"" << v->str << "\"})";
                break;

            case ValueTypeList:
                out << "new_value(List{";
                for(int i = 0; i < v->lst.size(); i++) {
                    write(v->lst[i]);
                    if(i != v->lst.size() - 1) out << ", ";
                }

                out << "})";
                break;

            default:
                break;
        }
    }

    void AstCppTranslator::write(AstNode node, int depth, bool endl) {
        switch(node->tag) {

            if(toplevel) write_mainstream = true;

            case AstTagValue:
                write(convert<_AstValue>(node)->value, 0);
                break;

            case AstTagOperation: {
                auto op = convert<_AstOperation>(node);
                if(op->op == "**") {
                    write("times(", depth); 
                    write(op->left, 0, false);
                    write(", ", 0);
                    write(op->right, 0, false);
                    write(")", 0);
                }
                else {
                    write(op->left, depth, false);
                    write(" " + op->op + " ", 0);
                    write(op->right, 0, false);
                }
                break;                      
            }

            case AstTagVariable:
                write(convert<_AstVariable>(node)->name, 0);
                break;

            case AstTagVariableAssignment: {
                auto assign = convert<_AstVariableAssignment>(node);
                write(assign->var_name + " = ", depth);
                write(assign->value, 0, false);
                break;
            }

            case AstTagList: {
                write("new_value(List{", depth);
                auto list = convert<_AstList>(node);
                for(int i = 0; i < list->elems.size(); i++) {
                    write(list->elems[i], depth + 4, false);
                    if(i != list->elems.size() - 1) out << ", ";
                };
                out << "})";
                break;
            }

            case AstTagListQ: {
                auto listq = convert<_AstListQ>(node);

                write(listq->name, 0);
                for(int i = 0; i < listq->indices.size(); i++)
                    out << "[",
                    write(listq->indices[i], 0, false),
                    out << "]";
                break;
            }

            case AstTagListAssignment: {
                auto list = convert<_AstListAssignment>(node);
                write(list->left_side, depth, false);
                write(" = ", 0);
                write(list->right_side, 0, false);
                break;
            }

            case AstTagReturn: {
                write("return ", depth);
                write(convert<_AstReturn>(node)->expr, 0, false);
                out << ";\n";
                break;
            }

            // TODO: XXX: implement fold expressions 
            case AstTagFoldExpression: {
                write("foldexpr:\n", depth);                           
                write("op = ", depth + 4);

                auto expr = convert<_AstFoldExpr>(node);

                write(expr->op, 0);
                out << "\n";

                for(int i = 0; i < expr->args.size(); i++)
                    write(expr->args[i], depth + 4, false);
                
                break;
            }

            case AstTagFunctionCall: {
                auto funcall = convert<_AstFunctionCall>(node);

                write(funcall->name + "(", depth);
                for(int i = 0; i < funcall->args.size(); i++) {
                    write(funcall->args[i], 0, false);
                    if(i != funcall->args.size() - 1) out << ", ";
                }
                write(")", 0);
                break;                         
            }


            case AstTagConditional: {
                auto cond = convert<_AstConditional>(node);                        

                for(int i = 0; i < cond->children.size(); i++) {
                    if(cond->children[i]->type == IfTypeIf) {
                        write("if((", depth);
                        write(cond->children[i]->expr, 0, false);
                        out << ").to_bool())";
                        write(cond->children[i]->block, depth + 4, true);
                    }

                    if(cond->children[i]->type == IfTypeElif) {
                        write("else if((", depth);
                        write(cond->children[i]->expr, depth + 4, false);
                        out << ").to_bool())";
                        write(cond->children[i]->block, depth + 4, true);                   
                    }

                    if(cond->children[i]->type == IfTypeElse) {
                        write("else", depth);
                        write(cond->children[i]->block, depth + 4, true);
                    }
                }

                return;
            }


            case AstTagWhile: {
                auto wh = convert<_AstWhile>(node);
                write("while((", depth);
                write(wh->condition, 0, false);
                write(").to_bool())", 0);
                write(wh->action, depth + 4, false);
                return;
            }

            case AstTagBlock: {
                auto block = convert<_AstBlock>(node);
                write("{\n", 0);
                for(int i = 0; i < block->children.size(); i++) {
                    write(block->children[i], depth, true);
                }
                write("}\n", depth - 4);
            
                return;
            }

            case AstTagFunctionDefinition: {
                auto fun = convert<_AstFunctionDefinition>(node);

                auto old_toplevel = toplevel;
                if(toplevel) write_mainstream = false;
                toplevel = false;

                write("Value ", depth);                               
                write(fun->name + "(", 0);

                for(int i = 0; i < fun->args.size(); i++) {
                    write("Value " + fun->args[i], 0);
                    if(i != fun->args.size() - 1) write(", ", 0);
                }

                out << ") ";
                write(fun->action, depth + 4, true);

                toplevel = old_toplevel;
                if(old_toplevel) write_mainstream = true;
                return;
            }

            case AstTagVariableDeclaration: {

                auto old_toplevel = toplevel;
                if(toplevel) write_mainstream = false;
                toplevel = false;

                write("Value ", depth);

                auto decl = convert<_AstVariableDeclaration>(node);
                for(int i = 0; i < decl->children.size(); i++) {
                    write(decl->children[i], 0, false);
                    if(i != decl->children.size() - 1)
                        out << ", ";
                }

                out << ";\n";
                toplevel = old_toplevel;
                if(old_toplevel) write_mainstream = true;

                return;
            }

            default: {
                break;
            }
        }

        if(endl) out << ";\n";
    }

    void AstCppTranslator::write_tree(AstNode n) {
        write(n, 0, true);
        std::cout << header << std::endl;
        std::cout << topstream.str() << std::endl;
        std::cout << "int main() " << mainstream.str() << std::endl;
    }    /* end AstWriter */
}


#include "byte.hpp"
    /* begin AstWriter */

#define out (to_stdout ? std::cout : stream)

#define new_node(name, type) type name = ptr::shared_ptr<_ ## type>();
#define new_astvalue(name) AstValue name = ptr::shared_ptr<_AstValue>(); \
                           name->tag = AstTagValue; \

   
namespace haxy {

    void AstWriter::write(Value v) {

        out << v->type << " ";

        switch(v->type) {
            case ValueTypeNumber:
            case ValueTypeBool:
                out << v->long_val;
                break;
            case ValueTypeDbl:
                stream << v->dbl;
                break;

            case ValueTypeString:
                stream << v->str;
                break;

            default:
                std::cerr << "Unexpected value given to AstWriter!" << std::endl;
                break;
        }

        out << " ";
    }
    void AstWriter::write(std::string str) {
        out << str << " ";
    }


    void AstWriter::write(AstNode node) {
        out << node->tag << " ";

        switch(node->tag) {
            case AstTagValue:
                write(node.convert<_AstValue>()->value);
                break;
            case AstTagVariable:
                out << node.convert<_AstVariable>()->name << " ";
                break;

            case AstTagList: {
                auto list = node.convert<_AstList>();

                out << list->elems.size() << " ";
                for(int i = 0; i < list->elems.size(); i++) write(list->elems[i]);
                break;
            }

            case AstTagListQ: {
                auto listq = node.convert<_AstListQ>();
                out << listq->name << " " << listq->indices.size() << " ";

                for(int i = 0; i < listq->indices.size(); i++)
                    write(listq->indices[i]);

                break;
            }

            case AstTagWhile: {
                auto wh = node.convert<_AstWhile>();
                write(wh->condition);
                write(wh->action);
                break;
            }

            case AstTagBlock: {
                auto block = node.convert<_AstBlock>();
                out << block->children.size() << " ";
                for(int i = 0; i < block->children.size(); i++)
                    write(block->children[i]);
            
                break;
            }

            case AstTagReturn: {
                write(node.convert<_AstReturn>()->expr);
                break;
            }

            case AstTagFunctionDefinition: {
                auto fun = node.convert<_AstFunctionDefinition>();
                out << fun->name << " " << fun->args.size() << " ";
                for(int i = 0; i < fun->args.size(); i++)
                    write(fun->args[i]);

                write(fun->action);
                break;
            }

            case AstTagAssignment: {
                auto assign = node.convert<_AstAssignment>();
                write(assign->left_side);
                write(assign->right_side);

                break;
            }

            case AstTagVariableDeclaration: {
                auto decl = node.convert<_AstVariableDeclaration>();

                out << decl->children.size() << " ";
                for(int i = 0; i < decl->children.size(); i++)
                    write(decl->children[i]);

                break;
            }

            case AstTagOperation: {
                auto expr = node.convert<_AstOperation>();

                out << expr->args.size() << " ";

                for(int i = 0; i < expr->args.size(); i++) {
                    write(expr->args[i]);
                    if(i != expr->args.size() - 1)
                        write(expr->ops[i]);
                }
                
                break;
            }

            case AstTagFunctionCall: {
                auto funcall = node.convert<_AstFunctionCall>();

                out << funcall->name << " " << funcall->args.size() << " ";
                for(int i = 0; i < funcall->args.size(); i++)
                    write(funcall->args[i]);
                break;                         
            }

            case AstTagConditional: {
                auto cond = node.convert<_AstConditional>();                        

                out << cond->children.size() << " ";
                for(int i = 0; i < cond->children.size(); i++) {

                    out << cond->children[i]->type << " ";
                    if(cond->children[i]->type == IfTypeIf) {
                        write(cond->children[i]->expr);
                        write(cond->children[i]->block);
                    }

                    if(cond->children[i]->type == IfTypeElif) {
                        write(cond->children[i]->expr);
                        write(cond->children[i]->block);                   
                    }

                    if(cond->children[i]->type == IfTypeElse) {
                        write(cond->children[i]->block);
                    }
                }
                break;
            }

            case AstTagClass: {
                auto cl = node.convert<_AstClass>();

                out << cl->name << " " << cl->vars.size() << " ";
                for(auto x : cl->vars)
                    write(x);

                out << cl->funcs.size() << " ";
                for(auto x : cl->funcs)
                    write(x);

                break;
            }

            case AstTagClassRef: {
                auto clref = node.convert<_AstClassReference>();                     

                out << clref->refs.size() << " ";
                for(auto x : clref->refs)
                    write(x);

                break;
            }

            case AstTagExpr: {
                out << " ";
                break;
            }
        }
    }

    void AstWriter::write_tree(AstNode n) {
        write(n);
    }

    /* end AstWriter */

    /* begin AstReader */

    AstNode AstReader::read_tree() {
        std::string label;

        return read();
    }

    Value AstReader::read_value() {

        Value v;

        ValueType type;
        int ttype;
        stream >> ttype;
        type = (ValueType)ttype;

        switch(type) {
            case ValueTypeNumber: {
                long tmp;
                stream >> tmp;
                v = new_value(tmp);
                break;
            }
            case ValueTypeBool: {
                long tmp;
                stream >> tmp;
                v = new_bvalue(tmp);
                break;
            }
            case ValueTypeDbl: {
                double tmp;
                stream >> tmp;
                v = new_dvalue(tmp);
                break;
            }

            case ValueTypeString: {
                std::string tmp;
                stream >> tmp;
                v = new_value(tmp);
                break;
            }

            default:
                v = new_error(BadOp);
                std::cerr << "Unexpected value given to AstReader!" << std::endl;
                break;
        }

        return v;
    }

    AstNode AstReader::read() {
        int ttag;
        stream >> ttag;

        struct Debug {
            ~Debug() {
                std::cout << " ***** " << std::endl;
            }
        } debug;

        std::cout << "read " << ttag << std::endl;

        AstTag tag = (AstTag)ttag;

        switch(tag) {
            case AstTagValue: {
                new_node(node, AstValue);
                node->tag = AstTagValue;
                node->value = read_value();
                return node;
            }
            case AstTagVariable: {
                new_node(node, AstVariable);
                node->tag = AstTagVariable;
                stream >> node->name;
                return node;
            }

            case AstTagList: {
                new_node(node, AstList);
                node->tag = AstTagList;

                int sz;
                stream >> sz;

                while(sz --> 0) node->elems.push_back(read());

                return node;
            }

            case AstTagListQ: {
                new_node(node, AstListQ);
                node->tag = AstTagListQ;

                stream >> node->name;

                int sz;
                stream >> sz;

                while(sz --> 0) node->indices.push_back(read());

                return node;
            }

            case AstTagWhile: {
                new_node(node, AstWhile);
                node->tag = AstTagWhile;

                node->condition = read();
                node->action = read().convert<_AstBlock>();

                return node;
            }

            case AstTagBlock: {
                new_node(node, AstBlock);
                node->tag = AstTagBlock;

                int sz;
                stream >> sz;
                while(sz --> 0) node->children.push_back(read());

                return node;
            }

            case AstTagReturn: {
                new_node(node, AstReturn);
                node->tag = AstTagReturn;
                node->expr = read();
                return node;
            }

            case AstTagFunctionDefinition: {
                new_node(node, AstFunctionDefinition);
                node->tag = AstTagFunctionDefinition;

                stream >> node->name;

                std::string tmp;
                int sz;
                stream >> sz;
                while(sz --> 0) stream >> tmp, node->args.push_back(tmp);

                node->action = read().convert<_AstBlock>();

                return node;
            }

            case AstTagAssignment: {
                new_node(node, AstAssignment);
                node->tag = AstTagAssignment;
                node->left_side = read();
                node->right_side = read();

                return node;
            }

            case AstTagVariableDeclaration: {
                new_node(node, AstVariableDeclaration);
                node->tag = AstTagVariableDeclaration;

                int sz;
                stream >> sz;
                while(sz --> 0)
                    node->children.push_back(read().convert<_AstDeclarableExpression>());

                return node;
            }

            case AstTagOperation: {
                new_node(node, AstOperation);
                node->tag = AstTagOperation;

                std::string op;

                int sz;
                stream >> sz;
                while(sz --> 0) {
                    node->args.push_back(read());
                    if(sz > 0)
                        stream >> op, node->ops.push_back(op);
                }
                
                return node;
            }

            case AstTagFunctionCall: {
                new_node(node, AstFunctionCall);
                node->tag = AstTagFunctionCall;

                stream >> node->name;

                int sz;
                stream >> sz;
                while(sz --> 0) node->args.push_back(read());

                return node;
            }

            case AstTagConditional: {
                new_node(node, AstConditional);
                node->tag = AstTagConditional;

                int sz;
                stream >> sz;
                while(sz --> 0) {

                    int ttype;
                    stream >> ttype;

                    new_node(child, AstIf);
                    child->type = (IfType) ttype;

                    switch(child->type) {
                        case IfTypeIf:
                        case IfTypeElif:
                            child->expr = read();
                            child->block = read().convert<_AstBlock>();
                            break;

                        case IfTypeElse: 
                            child->block = read().convert<_AstBlock>();
                            break;
                    }

                    node->children.push_back(child);
                }

                return node;
            }

            case AstTagClass: {
                new_node(node, AstClass);
                node->tag = AstTagClass;

                stream >> node->name;

                int sz;
                stream >> sz;
                while(sz --> 0)
                    node->vars.push_back(read().convert<_AstVariableDeclaration>());

                stream >> sz;
                while(sz --> 0)
                    node->funcs.push_back(read().convert<_AstFunctionDefinition>());

                return node;
            }

            case AstTagClassRef: {
                new_node(node, AstClassReference);
                node->tag = AstTagClassRef;

                int sz;
                stream >> sz;
                while(sz --> 0) node->refs.push_back(read());

                return node;
            }

            case AstTagExpr: {
                new_node(node, AstValue);
                node->tag = AstTagValue;
                node->value = new_error(NoValue);
                return node;
            }
        }
    }
}

#undef out

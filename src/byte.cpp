
#include "byte.hpp"
    /* begin AstWriter */

#define out (to_stdout ? std::cout : stream)
   
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
                stream << "\"" << v->str << "\"";
                break;

            default:
                std::cerr << "Unexpected value given to AstWriter!" << std::endl;
                break;
        }

        out << " ";
    }
    void AstWriter::write(std::string str) {
        out << str;
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

            case AstTagFoldExpression: {
                auto expr = node.convert<_AstFoldExpr>();

                out << expr->op << " " << expr->args.size() << " ";

                for(int i = 0; i < expr->args.size(); i++)
                    write(expr->args[i]);
                
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

            case AstTagOperation: {
                auto op = node.convert<_AstOperation>();
                out << op->op << " ";
                write(op->left);
                write(op->right);
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
    }

}

#undef out

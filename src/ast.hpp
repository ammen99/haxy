#ifndef AST_HPP
#define AST_HPP

#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <cstring>

#include "val.hpp"

extern "C" {
#include "mpc/mpc.h"
}

#define ast(x) (mpc_ast_t*)(x)
#define decl_shared_ptr(name) using name = ptr::shared_ptr< _ ## name>
std::string load_file(std::string name);
using AstNodeT = mpc_ast_t*;


namespace haxy {
        
    enum AstTag {
        AstTagValue    = 1,
        AstTagVariable = 2,
        AstTagList     = 3,
        AstTagListQ  = 4,
        AstTagExpr   = 5,
        AstTagWhile  = 6,
        AstTagBlock  = 7,
        AstTagReturn = 8,
        AstTagFunctionDefinition  = 9,
        AstTagAssignment          = 10,
        AstTagVariableDeclaration = 11,
        AstTagFoldExpression      = 13,
        AstTagFunctionCall        = 14,
        AstTagConditional         = 15,
        AstTagOperation           = 16,
        AstTagClass               = 17,
        AstTagClassRef            = 18,
    };

    struct _AstNode {
        AstTag tag;
    };
    decl_shared_ptr(AstNode);

    struct _AstExpr : _AstNode {};
    decl_shared_ptr(AstExpr);

    struct _AstBlock : _AstNode {
        std::vector<AstNode> children; 
    };
    decl_shared_ptr(AstBlock);

    struct _AstValue : _AstExpr {
        Value value; 
    };
    decl_shared_ptr(AstValue);

    struct _AstOperation : _AstExpr {
        std::string op;
        AstNode left, right; 
    };
    decl_shared_ptr(AstOperation);

    struct _AstFoldExpr : _AstExpr {
        std::string op; 
        std::vector<AstNode> args;
    };
    decl_shared_ptr(AstFoldExpr);

    struct _AstFunctionCall : _AstExpr {
        std::string name;
        std::vector<AstNode> args;
    };
    decl_shared_ptr(AstFunctionCall);

    struct _AstList : _AstExpr {
        std::vector<AstNode> elems; 
    };
    decl_shared_ptr(AstList);

    struct _AstListQ : _AstExpr {
        std::string name;
        std::vector<AstNode> indices;
    };
    decl_shared_ptr(AstListQ);

    struct _AstDeclarableExpression : _AstExpr {};
    decl_shared_ptr(AstDeclarableExpression);

    struct _AstVariable : _AstDeclarableExpression {
        std::string name; 
    };
    decl_shared_ptr(AstVariable);

    struct _AstAssignment : _AstDeclarableExpression {
        AstNode left_side; 
        AstNode right_side;
    };

    decl_shared_ptr(AstAssignment);


    struct _AstVariableDeclaration : _AstNode {
        std::vector<AstDeclarableExpression> children; 
    };
    decl_shared_ptr(AstVariableDeclaration);
    
    struct _AstReturn : _AstNode {
        AstNode expr; 
    };
    decl_shared_ptr(AstReturn);

    struct _AstFunctionDefinition : _AstNode {
        std::string name;
        std::vector<std::string> args;
        AstBlock action;
    };
    decl_shared_ptr(AstFunctionDefinition);

    struct _AstClass : _AstNode {
        std::string name;
        std::vector<AstVariableDeclaration> vars;
        std::vector<AstFunctionDefinition> funcs;
    };
    decl_shared_ptr(AstClass);

    enum IfType {
        IfTypeIf   = 1,
        IfTypeElif = 2,
        IfTypeElse = 3
    };

    struct _AstIf : _AstNode {
        IfType type;
        AstNode expr; 
        AstBlock block;
    };
    decl_shared_ptr(AstIf);
    
    struct _AstConditional : _AstNode {
        std::vector<AstIf> children; 
    };
    decl_shared_ptr(AstConditional);

    struct _AstWhile : _AstNode {
        AstNode condition;
        AstBlock action;
    };
    decl_shared_ptr(AstWhile);

    struct _AstClassReference : _AstNode {
        std::vector<AstNode> refs; 
    };
    decl_shared_ptr(AstClassReference);


    class AstGenerator {
        AstGenerator() = delete;

        static std::vector<AstNode> parse_args(AstNodeT);          //

        static AstNode  generate_while(AstNodeT);      //
        static AstNode  generate_if(AstNodeT);         //
        static AstBlock generate_block(AstNodeT);      //
        static AstNode  generate_list(AstNodeT);       //

        static AstVariableDeclaration generate_vardecl (AstNodeT);
        static AstFunctionDefinition  generate_func_def(AstNodeT);  //

        static AstNode generate_func_call(AstNodeT);  //
        static AstNode generate_comp(AstNodeT);       //
        static AstNode generate(AstNodeT);            //

        public:
        static AstNode parse_file(AstNodeT toplevel);
    };

    class AstPrinter {
        std::fstream stream;
        bool to_stdout;

        void write(std::string str, int depth = 0);
        void write(Value v, int depth = 0);
        void write(AstNode node, int depth);

        public:
        AstPrinter() :to_stdout(true) {}
        AstPrinter(std::string name) : to_stdout(false) { stream.open(name); }

        void write_tree(AstNode root);

        ~AstPrinter() { if(!to_stdout) stream.close(); }
    };

}

#endif

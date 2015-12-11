#ifndef AST_HPP
#define AST_HPP

#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <cstring>

#include "val.hpp"
#include "mpc.h"

#define ast(x) (mpc_ast_t*)(x)
#define decl_shared_ptr(name) using name = std::shared_ptr< _ ## name>
std::string load_file(std::string name);
using AstNodeT = mpc_ast_t*;


namespace haxy {
        
        
        enum AstTag {
        AstTagValue    = 1, //
        AstTagVariable = 2, //
        AstTagList     = 3, //
        AstTagListQ  = 4,   //
        AstTagExpr   = 5,   //
        AstTagWhile  = 6,   //
        AstTagBlock  = 7,   //
        AstTagReturn = 8,   //
        AstTagFunctionDefinition  = 9,  //
        AstTagVariableAssignment  = 10, //
        AstTagVariableDeclaration = 11, //
        AstTagListAssignment      = 12, //
        AstTagFoldExpression      = 13, //
        AstTagFunctionCall        = 14, //
        AstTagConditional         = 15, //
        AstTagOperation           = 16, //
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

    struct _AstListAssignment : _AstNode {
        AstListQ left_side; 
        AstNode right_side;
    };

    decl_shared_ptr(AstListAssignment);

    struct _AstDeclarableExpression : _AstExpr {};
    decl_shared_ptr(AstDeclarableExpression);

    struct _AstVariable : _AstDeclarableExpression {
        std::string name; 
    };
    decl_shared_ptr(AstVariable);

    struct _AstVariableAssignment : _AstDeclarableExpression {
        std::string var_name;
        AstNode value;
    };
    decl_shared_ptr(AstVariableAssignment);

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


    class AstGenerator {
        AstGenerator() = delete;

        static std::vector<AstNode> parse_args(AstNodeT);          //

        static AstNode generate_while(AstNodeT);      //
        static AstNode generate_if(AstNodeT);         //
        static AstBlock generate_block(AstNodeT);      //
        static AstNode generate_list(AstNodeT);       //
        static AstNode generate_func_def (AstNodeT);  //
        static AstNode generate_func_call(AstNodeT);  //
        static AstNode generate_comp(AstNodeT);       //
        static AstNode generate(AstNodeT);            //

        public:
        static AstNode parse_file(AstNodeT toplevel);
    };

    template<class T> std::shared_ptr<T> convert(AstNode node);

    class AstWriter {
        std::fstream stream;
        bool to_stdout;

        void write(std::string str, int depth = 0);
        void write(Value v, int depth = 0);
        void write(AstNode node, int depth);

        public:
        AstWriter() :to_stdout(true) {}
        AstWriter(std::string name) : to_stdout(false) { stream.open(name); }

        void write_tree(AstNode root);

        ~AstWriter() { if(!to_stdout) stream.close(); }
    };

    class AstCppTranslator {
        std::ostringstream mainstream, topstream;
        bool toplevel = true;
        bool write_mainstream = true;

        void write(std::string str, int depth = 0);
        void write(Value v, int depth = 0);
        void write(AstNode node, int depth, bool endl);

        std::string header = "#include <builtin.hpp>";

        public:
        AstCppTranslator() {
        }

        void write_tree(AstNode node);
    };
}

#endif

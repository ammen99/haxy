#include <sstream>
#include "ast.hpp"

namespace haxy {
    class AstWriter {
        std::fstream stream;
        bool to_stdout;

        void write(std::string str);
        void write(Value v);
        void write(AstNode node);

        public:
        AstWriter() :to_stdout(true) {}
        AstWriter(std::string name) : to_stdout(false) { stream.open(name, std::ios::out); }

        void write_tree(AstNode root);

        ~AstWriter() { if(!to_stdout) stream.close(); }
    };

    class AstReader {
        std::string src;
        std::stringstream stream;

        Value read_value();
        AstNode read();

        public:
        AstReader(std::string s) : src(s) { stream.str(src); } 
        AstNode read_tree();
    };
}

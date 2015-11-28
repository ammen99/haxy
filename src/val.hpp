#ifndef VAL_HPP
#define VAL_HPP

#define MAXINT 2000000000

#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <list>

enum ValueType { 
    ValueTypeNumber = 1 << 0,
    ValueTypeDbl    = 1 << 1,
    ValueTypeString = 1 << 2,
    ValueTypeList   = 1 << 3,
    ValueTypeBool   = 1 << 4,
    ValueTypeError  = 1 << 5 };

#define ValueTypeArithmetic (ValueTypeNumber|ValueTypeDbl)
#define ValueTypeBoolArithm (ValueTypeNumber|ValueTypeBool)

enum Error { ZeroDiv, BadOp, BadValue, UnknownSym, NoValue, ArgumentNumberMismatch };

struct Value;

using List = std::vector<Value>;
using String = std::string;

struct Value {
    ValueType type;

    union {
        Error error;
        long long_val;
        double dbl;
        List lst;
        String str;
    };
    
    bool return_value = false;

    Value();
    Value(const Value&  other);
    Value(const Value&& other);
    Value operator = (const Value&  other);
    Value operator = (const Value&& other);
    ~Value();
};

Value new_value(long x);
Value new_value(List l);
Value new_value(String str);

Value new_dvalue(double d);
Value new_bvalue(bool b);
Value new_error (Error e);

std::ostream& operator << (std::ostream& stream, const Value& v);

Value operator + (const Value &a, const Value &b);
Value operator - (const Value &a, const Value &b);
Value operator * (const Value &a, const Value &b);
Value operator / (const Value &a, const Value &b);

Value operator && (const Value &a, const Value &b);
Value operator || (const Value &a, const Value &b);

Value operator == (const Value &a, const Value &b);
Value operator >= (const Value &a, const Value &b);
Value operator <= (const Value &a, const Value &b);
Value operator != (const Value &a, const Value &b);
Value operator >  (const Value &a, const Value &b);
Value operator <  (const Value &a, const Value &b);

Value times(const Value &a, const Value &b);


using Args = std::vector<Value>;

struct Func {
    size_t min_arg;
    size_t max_arg;

    std::function<Value(Args)> call;

    bool eval_args_by_identifier;
};

bool is_computable(char *str);


#endif /* end of include guard: VAL_HPP */

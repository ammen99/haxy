/* This file contains the *standard library* for haxy */
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

enum Error { ZeroDiv, BadOp, BadValue, UnknownSym, NoValue, ArgumentNumberMismatch, IndexingNonArray, ArraySubscriptNotAnInteger };

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

    bool to_bool();

    Value();
    Value(const Value&  other);
    Value(const Value&& other);
    Value operator = (const Value&  other);
    Value operator = (const Value&& other);
    ~Value();

    Value& operator [] (Value index);
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
Value operator % (const Value &a, const Value &b);

Value operator && (const Value &a, const Value &b);
Value operator || (const Value &a, const Value &b);

Value operator == (const Value &a, const Value &b);
Value operator >= (const Value &a, const Value &b);
Value operator <= (const Value &a, const Value &b);
Value operator != (const Value &a, const Value &b);
Value operator >  (const Value &a, const Value &b);
Value operator <  (const Value &a, const Value &b);

Value times(const Value &a, const Value &b);

void raise_error(Error e);

using Args = std::vector<Value>;

struct Func {
    size_t min_arg;
    size_t max_arg;

    std::function<Value(Args)> call;

    bool eval_args_by_identifier;
};

bool is_computable(char *str);

void print(Value v);
template<class... Args> void print(Value v, Args ... args);

#endif /* end of include guard: VAL_HPP */


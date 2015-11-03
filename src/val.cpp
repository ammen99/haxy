#include "val.hpp"
#include <cstring>
#include <new>

/* constructors/destructors/assignment operators */
/* satisfying the rule of five */

Value::Value() {
    type = ValueTypeError;
    error = NoValue;
}

Value::Value(const Value &other) {
    type = other.type;
    switch(type) {
        case ValueTypeNumber:
        case ValueTypeBool:
            long_val = other.long_val;
            break;
        case ValueTypeDbl:
            dbl = other.dbl;
            break;
        case ValueTypeList:
            new(&lst) List(other.lst);
            break;
        case ValueTypeString:
            new(&str) std::string(other.str);
            break;
        case ValueTypeError:
            error = other.error;
            break;
    }
}

Value::Value(const Value &&other) {
    type = other.type;
    switch(type) {
        case ValueTypeNumber:
        case ValueTypeBool:
            long_val = other.long_val;
            break;
        case ValueTypeDbl:
            dbl = other.dbl;
            break;
        case ValueTypeList:
            new(&lst) List(std::move(other.lst));
            break;
        case ValueTypeString:
            new(&str) String(std::move(other.str));
            break;
        case ValueTypeError:
            error = other.error;
            break;
    }
}

Value::~Value() {
    if(type == ValueTypeList)
        lst.~vector();

    if(type == ValueTypeString)
        str.~basic_string();
}

Value Value::operator=(const Value &other) {
    if(this != &other) {
        type = other.type;
        switch(type) {
            case ValueTypeNumber:
            case ValueTypeBool:
                long_val = other.long_val;
                break;
            case ValueTypeDbl:
                dbl = other.dbl;
                break;
            case ValueTypeList:
                lst = other.lst;
                break;
            case ValueTypeString:
                str = other.str;
                break;
            case ValueTypeError:
                error = other.error;
                break;
        }    
    }
    return *this;
}

Value Value::operator=(const Value &&other) {
    if(this != &other) {
        type = other.type;
        switch(type) {
            case ValueTypeNumber:
            case ValueTypeBool:
                long_val = other.long_val;
                break;
            case ValueTypeDbl:
                dbl = other.dbl;
                break;
            case ValueTypeList:
                lst = std::move(other.lst);
                break;
            case ValueTypeString:
                str = std::move(other.str);
                break;
            case ValueTypeError:
                error = other.error;
                break;
        }    
    }

    return *this;
}

Value new_value(long x) {
    Value v;
    v.type = ValueTypeNumber;
    v.long_val = x;

    return v;
}

Value new_value(List l) {
    std::cout << "Creating value from list" << std::endl;
    Value v;
    std::cout << "Blah blah " << std::endl;
    v.type = ValueTypeList;
    std::cout << "SSS" << l.size() << std::endl;
    new(&v.lst) List(std::move(l));

    std::cout << "in std::move" << std::endl;
    return v;
}

Value new_bvalue(bool b) {
    Value v;
    v.type = ValueTypeBool;
    v.long_val = b ? 1 : 0;

    return v;
}

Value new_value(String str) {

    Value v;
    v.type = ValueTypeString;

    std::cout << "slow crappy thing" << std::endl;
    new(&v.str) String(std::move(str));
    std::cout << "slower crappy thing" << std::endl;

    return v;
}

Value new_dvalue(double d) {
    Value v;
    v.type = ValueTypeDbl;
    v.dbl = d;

    return v;
}

Value new_error(Error e) {
    Value v;
    v.type = ValueTypeError;
    v.error = e;

    return v;
}

namespace {
    std::string getStringError(Error e) {
        if(e == ZeroDiv)
            return "ZeroDiv"; 

        if(e == BadOp)
            return "BadOp";

        if(e == BadValue)
            return "BadValue";
        
        if(e == UnknownSym)
            return "Undefined variable/function";

        if(e == NoValue)
            return "No Value";

        if(e == ArgumentNumberMismatch)
            return "Too few/much arguments to call function";

        return "Unknown error";
    }
}

std::ostream& operator << (std::ostream &stream, const Value &v) {

    if(v.type == ValueTypeError && v.error != NoValue)
        stream << "Error: " << getStringError(v.error); 

    else if(v.type == ValueTypeError) { }
    else if(v.type == ValueTypeList) {
        stream << "[";

        auto size = v.lst.size();
        for(size_t i = 0; i < size; ++i) 
            if(i < size - 1)
                stream << v.lst[i] << ", ";
            else
                stream << v.lst[i];

        stream << "]";
    }
    else if(v.type == ValueTypeBool) {
        if(v.long_val)
            stream << "true";
        else
            stream << "false";
    }

    else if(v.type == ValueTypeString)
        stream << "\"" << v.str << "\"";

    else if(v.type == ValueTypeDbl)
        stream << v.dbl;
    else
        stream << v.long_val;
    
    return stream;
}


#define operator_guard(a, b) if(a.type == ValueTypeError) \
                                return a; \
                             if(b.type == ValueTypeError) \
                                return b;

/* arithmetic operators */


/* concatenate to lists */
Value concat(const Value &a, const Value &b) {
    auto res = new_value(a.lst);

    for(auto x : b.lst)
        res.lst.push_back(x);

    return res;
}

/* append a list to a value */
Value append(const Value &a, const Value &b) {
    auto res = new_value(a.lst);
    res.lst.push_back(b);
    return res;
}

/* insert a value in front of the list */
Value insertFront(const Value &a, const Value &b) {
    auto res = new_value(b.lst);
    res.lst.insert(res.lst.begin(), a);
    return res;
}

#define arithmetic_op_guard(a, b, op) \
    if((a.type & ValueTypeArithmetic) && \
        (b.type & ValueTypeArithmetic)) { \
          \
        if(a.type == ValueTypeDbl && b.type == ValueTypeDbl) \
            return new_dvalue(a.dbl op b.dbl); \
        else if(a.type == ValueTypeDbl) \
            return new_dvalue(a.dbl op double(b.long_val)); \
        else if(b.type == ValueTypeDbl) \
            return new_dvalue(double(a.long_val) op b.dbl); \
        else \
            return new_value(a.long_val op b.long_val); \
    }


/* appends one list to another if lists, else just sum */
Value operator + (const Value &a, const Value &b) {
    operator_guard(a, b);

    if(a.type == ValueTypeList && b.type == ValueTypeList)
        return concat(a, b);
    else if(a.type == ValueTypeList)
        return append(a, b);
    else if(b.type == ValueTypeList)
        return insertFront(a, b);

    arithmetic_op_guard(a, b, +)
    else if(b.type == ValueTypeString)
        return new_value(a.str + b.str);
    else
        return new_error(BadOp);
}

Value operator - (const Value &a, const Value &b) {
    operator_guard(a, b);

    arithmetic_op_guard(a, b, -)
    else
        return new_error(BadOp);
}

Value operator * (const Value &a, const Value &b) {
    operator_guard(a, b);

    arithmetic_op_guard(a, b, *)
    else if(a.type == ValueTypeList && b.type == ValueTypeNumber) {
        std::cout << "here" << std::endl;

        Value v = new_value(List{});
        for(int i = 0; i < b.long_val; i++)
            v.lst.push_back(a);
        std::cout << "Zu ende" << std::endl;

        return v;
    }
    else
        return new_error(BadOp);
}

#define abs(x) (x) < 0 ? (-(x)) : (x)

Value operator / (const Value &a, const Value &b) {
    operator_guard(a, b);

    if((b.type == ValueTypeNumber && b.long_val == 0) ||
        (b.type == ValueTypeDbl   && abs(b.dbl) < 1e-15))
        return new_error(ZeroDiv);

    arithmetic_op_guard(a, b, /)
    else
        return new_error(BadOp);
}

#define logic_operator_guard(a, b) \
    if(!(a.type & ValueTypeBoolArithm) || (!b.type & ValueTypeBoolArithm)) \
        return new_error(BadOp);

/* boolean operators */
Value operator && (const Value &a, const Value &b) {
    logic_operator_guard(a, b);
    return new_bvalue(a.long_val && b.long_val);
}
Value operator || (const Value &a, const Value &b) {
    logic_operator_guard(a, b);
    return new_bvalue(a.long_val || b.long_val);
}

#define compare_op_guard(a, b, op) \
    if((a.type & ValueTypeArithmetic) && \
        (b.type & ValueTypeArithmetic)) { \
          \
        if(a.type == ValueTypeDbl && b.type == ValueTypeDbl) \
            return new_bvalue(a.dbl op b.dbl); \
        else if(a.type == ValueTypeDbl) \
            return new_bvalue(a.dbl op double(b.long_val)); \
        else if(b.type == ValueTypeDbl) \
            return new_bvalue(double(a.long_val) op b.dbl); \
        else \
            return new_bvalue(a.long_val op b.long_val); \
    } else if(a.type == ValueTypeString && b.type == ValueTypeString) \
        return new_bvalue(a.str op b.str);


// TODO: implement comparison for lists and strings 
/* comparison operator */
Value operator == (const Value &a, const Value &b) {
    compare_op_guard(a, b, ==)
    else return new_error(BadOp);
}
Value operator >= (const Value &a, const Value &b) {
    compare_op_guard(a, b, >=)
    else return new_error(BadOp);
}
Value operator <= (const Value &a, const Value &b) {
    compare_op_guard(a, b, <=)
    else return new_error(BadOp);
}
Value operator != (const Value &a, const Value &b) {
    compare_op_guard(a, b, !=)
    else return new_error(BadOp);
}
Value operator > (const Value &a, const Value &b) {
    compare_op_guard(a, b, >)
    else return new_error(BadOp);
}
Value operator < (const Value &a, const Value &b) {
    compare_op_guard(a, b, <)
    else return new_error(BadOp);
}

bool is_computable(char * str) {
    if(std::strstr(str, "expr")) return true;
    if(std::strstr(str, "func")) return true;
    if(std::strstr(str, "var")) return true;
    if(std::strstr(str, "number")) return true;
    if(std::strstr(str, "bool")) return true;

    return false;
}

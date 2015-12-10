#include "builtin.hpp"
#include <cstring>
#include <new>

void print(Value v) {
    std::cout << v << std::endl;
}

template<class... Args> void print(Value v, Args ... args) {
    std::cout << v << " "; 
    print(args...);
}

/* TODO: exit when raising an error */
void raise_error(Error e) {
    std::cout << "Critical error!" << std::endl;
}

/* constructors/destructors/assignment operators */
/* satisfying the rule of five */

Value::Value() {
    type = ValueTypeError;
    error = NoValue;
}

Value::Value(const Value &other) {
    type = other.type;
    return_value = other.return_value;
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
    return_value = other.return_value;
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
    switch(type) {
        case ValueTypeList:
            lst.~vector();
            break;

        case ValueTypeString:
            str.~basic_string();
            break;

        default:
            break;
    }
}

Value Value::operator=(const Value &other) {
    if(this != &other) {
        type = other.type;
        return_value = other.return_value;
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
                new(&str) String(other.str);
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
        return_value = other.return_value;
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

    return *this;
}

Value no_value = new_error(NoValue);

Value& Value::operator[] (Value index) {
    if(index.type != ValueTypeNumber)
        raise_error(ArraySubscriptNotAnInteger);

    switch(type) {
        case ValueTypeList:
            return lst[index.long_val];
        case ValueTypeString:
        default:
            raise_error(IndexingNonArray);
            return no_value;
    }
}

Value new_value(long x) {
    Value v;
    v.type = ValueTypeNumber;
    v.long_val = x;

    return v;
}

Value new_value(List l) {
    Value v;
    v.type = ValueTypeList;
    new(&v.lst) List(std::move(l));

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
    new(&v.str) String(std::move(str));

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
        switch(e) {
            case ZeroDiv:
                return "ZeroDiv"; 

            case BadOp:
                return "BadOp";

            case BadValue:
                return "BadValue";

            case UnknownSym:
                return "Undefined variable/function";

            case NoValue:
                return "No Value";

            case ArgumentNumberMismatch:
                return "Too few/much arguments to call function";

            default:
                return "Unknown error";
        }
    }
}

std::ostream& operator << (std::ostream &stream, const Value &v) {

    switch(v.type) {
        case ValueTypeNumber:
            stream << v.long_val;
            break;
        case ValueTypeDbl:
            stream << v.dbl;
            break;
        case ValueTypeError:
            if(v.error != NoValue)
                stream << "Error: " << getStringError(v.error); 

            break;

        case ValueTypeList: {
            stream << "[";

            auto size = v.lst.size();
            for(size_t i = 0; i < size; ++i) 
                if(i < size - 1)
                    stream << v.lst[i] << ", ";
                else
                    stream << v.lst[i];

            stream << "]";
            break;
        }

        case ValueTypeBool:
            if(v.long_val)
                stream << "true";
            else
                stream << "false";
            break;

        case ValueTypeString:
            stream << "\"" << v.str << "\"";
    }

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
    else
        return new_error(BadOp);
}

Value times(const Value &a, const Value &b) {
    if(b.type == ValueTypeNumber) {
        Value v = new_value(List{}); 
        auto len = b.long_val;
        for(int i = 0; i < len; i++)
            v.lst.push_back(a);

        return v;
    }
    else return new_error(BadOp);
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

Value operator % (const Value &a, const Value &b) {
    if(a.type == ValueTypeNumber && b.type == ValueTypeNumber) {
        if(b.long_val == 0) return new_error(ZeroDiv); 
        else return new_value(a.long_val % b.long_val);
    }

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
    switch((a.type << 3) | b.type) { \
        case 9: \
            return new_bvalue(a.long_val op b.long_val); \
        case 10: \
            return new_bvalue(a.long_val op b.dbl); \
        case 17: \
            return new_bvalue(a.dbl op b.long_val); \
        case 18: \
            return new_bvalue(a.dbl op b.dbl); \
        case 72: \
            return new_bvalue(a.str op b.str); \
        default: \
            return new_error(BadOp); \
    }


// TODO: implement comparison for lists and strings 
/* comparison operator */
Value operator == (const Value &a, const Value &b) {
    compare_op_guard(a, b, ==)
}
Value operator >= (const Value &a, const Value &b) {
    compare_op_guard(a, b, >=)
}
Value operator <= (const Value &a, const Value &b) {
    compare_op_guard(a, b, <=)
}
Value operator != (const Value &a, const Value &b) {
    compare_op_guard(a, b, !=)
}
Value operator > (const Value &a, const Value &b) {
    compare_op_guard(a, b, >)
}
Value operator < (const Value &a, const Value &b) {
    compare_op_guard(a, b, <)
}

bool is_computable(char * str) {
    if(std::strstr(str, "expr")) return true;
    if(std::strstr(str, "func")) return true;
    if(std::strstr(str, "var")) return true;
    if(std::strstr(str, "number")) return true;
    if(std::strstr(str, "bool")) return true;

    return false;
}

bool Value::to_bool() {
    switch(type) {
        case ValueTypeNumber:
        case ValueTypeBool:
            return long_val;
        
        case ValueTypeDbl:
            return abs(dbl) > 1e-16;
        
        case ValueTypeString:
            return !str.size();
        
        case ValueTypeList:
            return !lst.size();

        case ValueTypeError:
            return false;
    }
}

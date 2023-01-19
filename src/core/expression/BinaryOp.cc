
#include <cstdint>
#include <cassert>
#include <utils/printutils.h>
#include <utils/exceptions.h>
#include <core/Value.h>
#include <core/Context.h>
#include <core/expression/BinaryOp.h>

BinaryOp::BinaryOp(Expression *left, BinaryOp::Op op, Expression *right, const Location& loc) :
  Expression(Id::BinaryOp,loc), op(op), left(left), right(right){}

namespace {

   Value checkUndef(Value&& val, Location const & loc, std::string const  & doc)
   {
      if (val.isUncheckedUndef()) {
         LOG(message_group::Warning, loc, doc,
          "%1$s", val.toUndefString());
      }
      return val;
   };

   using binOpFun = Value(Value&&, Value&& );
   struct binOpInfo_t{
      constexpr binOpInfo_t(const char str[], binOpFun f) : opStr{str},fun{f}{};
      const char * opStr;
      const binOpFun * const fun;
   };

   // These indices must match the BinaryOp::Op enums
   // converted to int as defined in the BinaryOp class
   constexpr binOpInfo_t  binOps[] = {
       // N.B the logical op functions && || arent called
       {  "&&", [](Value&& lhs, Value&& rhs)->Value { return lhs.toBool() && rhs.toBool(); } }// LogicalAnd
      ,{  "||", [](Value&& lhs, Value&& rhs)->Value { return lhs.toBool() || rhs.toBool(); } }// LogicalOr
      //--------------------------------------------------------------
      ,{   "^", [](Value&& lhs, Value&& rhs)->Value { return lhs ^ rhs; }  } // Exponent
      ,{   "*", [](Value&& lhs, Value&& rhs)->Value { return lhs * rhs; }  } // Multiply
      ,{   "/", [](Value&& lhs, Value&& rhs)->Value { return lhs / rhs; }  } // Divide
      ,{   "%", [](Value&& lhs, Value&& rhs)->Value { return lhs % rhs; }  } // Modulo
      ,{   "+", [](Value&& lhs, Value&& rhs)->Value { return lhs + rhs; }  } // Plus
      ,{   "-", [](Value&& lhs, Value&& rhs)->Value { return lhs - rhs; }  } // Minus
      ,{   "<", [](Value&& lhs, Value&& rhs)->Value { return lhs < rhs; }  } // Less
      ,{  "<=", [](Value&& lhs, Value&& rhs)->Value { return lhs <= rhs; } } // LessEqual
      ,{   ">", [](Value&& lhs, Value&& rhs)->Value { return lhs > rhs; }  }  // Greater
      ,{  ">=", [](Value&& lhs, Value&& rhs)->Value { return lhs >= rhs; } }  // GreaterEqual
      ,{  "==", [](Value&& lhs, Value&& rhs)->Value { return lhs == rhs; } } // Equal
      ,{  "!=", [](Value&& lhs, Value&& rhs)->Value { return lhs != rhs; } } // NotEqual
      ,{  "->", [](Value&& lhs, Value&& rhs)->Value { return Value::undefined.clone(); } }  // Translate
      ,{  "^>", [](Value&& lhs, Value&& rhs)->Value { return Value::undefined.clone(); } }  // Rotate
   };

   void exitBinopFail() {
      assert(false && "Non-existent binary operator!");
      throw EvaluationException("Non-existent binary operator!");
   }

   [[nodiscard]] constexpr inline bool binOpIndexInRange(int index)
   {
      return (index >= 0) && (index <= static_cast<int>(BinaryOp::Op::Rotate));
   }
}

Value BinaryOp::evaluate(const std::shared_ptr<const Context>& context) const
{
   switch  (auto const opId = this->op) {
      default:{
         return checkUndef(
            binOps[static_cast<int>(opId)].fun(
               this->left->evaluate(context),
               this->right->evaluate(context)
            ),
            this->location(), context->documentRoot()
         );
      }
      // These two ops are shortcircuit evaluated in C++, so both sides of the expression
      // arent necessarily evaluated
      case Op::LogicalAnd:
         return this->left->evaluate(context).toBool() && this->right->evaluate(context).toBool();
      case Op::LogicalOr:
         return this->left->evaluate(context).toBool() || this->right->evaluate(context).toBool();
   }
}

const char *BinaryOp::opString() const
{
   return binOps[static_cast<int>(this->op)].opStr;
}

void BinaryOp::print(std::ostream& stream, const std::string&) const
{
  stream << "(" << *this->left << " " << opString() << " " << *this->right << ")";
}

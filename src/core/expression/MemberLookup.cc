
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision module_reference member lookup
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include <typeinfo>
#include <boost/regex.hpp>

#include <core/value/ModuleReference.h>
#include <core/Value.h>
#include <core/UserModule.h>
#include <core/Arguments.h>
#include <core/UserModuleContext.h>

#include <core/expression/MemberLookup.h>

MemberLookup::MemberLookup(Expression *expr, const std::string& member, const Location& loc)
  : Expression(Id::MemberLookup,loc), expr(expr), member(member) {}

Value MemberLookup::evaluate(const std::shared_ptr<const Context>& context) const
{
  const Value& v = this->expr->evaluate(context);
  static const boost::regex re_swizzle_validation("^([xyzw]{1,4}|[rgba]{1,4})$");

  switch (v.type()) {
  case Value::Type::VECTOR:
    if (this->member.length() > 1 && boost::regex_match(this->member, re_swizzle_validation)) {
      VectorType ret(context->session());
      for (const char& ch : this->member)
        switch (ch) {
        case 'r': case 'x': ret.emplace_back(v[0]); break;
        case 'g': case 'y': ret.emplace_back(v[1]); break;
        case 'b': case 'z': ret.emplace_back(v[2]); break;
        case 'a': case 'w': ret.emplace_back(v[3]); break;
        }
      return Value(std::move(ret));
    }
    if (this->member == "x") return v[0];
    if (this->member == "y") return v[1];
    if (this->member == "z") return v[2];
    if (this->member == "w") return v[3];
    if (this->member == "r") return v[0];
    if (this->member == "g") return v[1];
    if (this->member == "b") return v[2];
    if (this->member == "a") return v[3];
    break;
  case Value::Type::RANGE:
    if (this->member == "begin") return v[0];
    if (this->member == "step") return v[1];
    if (this->member == "end") return v[2];
    break;
  case Value::Type::OBJECT:
    return v[this->member];
  case Value::Type::MODULE:{

     auto const & modRef = v.toModuleReference();
     std::shared_ptr<const Context> module_lookup_context = modRef.getContext();
     ModuleReference const * pModRef = &modRef;
     AssignmentList arguments;
     for (;;){

      AssignmentList argsTemp;
      if (pModRef->transformToInstantiationArgs(
         arguments,
         this->loc,
         context,
         argsTemp
      )){
         arguments = argsTemp;
      }else{
         return Value::undefined.clone();
      }

      boost::optional<InstantiableModule> iModule =
         module_lookup_context->lookup_module(pModRef->getModuleName(), this->loc);
      if (iModule) {
         auto user_module = dynamic_cast<const UserModule*>(iModule->module);
         // iModule->module  :  AbstractModule *  ==  UserModule * | BuiltinModule *
         if ( user_module){
            // push on static stack, pop at end of method!
            StaticModuleNameStack name{pModRef->getModuleName()};
            ContextHandle<UserModuleContext> module_context{
               Context::create<UserModuleContext>(
                  iModule->defining_context,
                  user_module,
                  this->location(),
                  Arguments(arguments, pModRef->getContext()),
                  Children(&user_module->body, pModRef->getContext())
               )
            };
            auto maybe_value = module_context->lookup_local_variable( this->member);
            if (maybe_value){
              return std::move(maybe_value->clone());
            }
          } else{
            //TODO
            // auto builtin_module = dynamic_cast<const BuiltinModule*>(iModule->module)
             return Value::undefined.clone();
          }
      }else{
        boost::optional<const Value&> maybe_modRef = module_lookup_context->lookup_moduleReference(pModRef->getModuleName());
        if ( static_cast<bool>(maybe_modRef) == true){
           auto const & modRef = maybe_modRef->toModuleReference();
           module_lookup_context = modRef.getContext();
           pModRef = & modRef;
        }else{
          return Value::undefined.clone();
        }
      }
     }
  }

  default:
    break;
  }
  return Value::undefined.clone();
}

void MemberLookup::print(std::ostream& stream, const std::string&) const
{
  stream << *this->expr << "." << this->member;
}

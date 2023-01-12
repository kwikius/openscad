#pragma once

#include <memory>

#include <Feature.h>

class AbstractNode;
class Context;
class ModuleInstantiation;

/**
* @brief abstract base class for UserModule and BuiltinModule
**/
class AbstractModule
{
public:

   using abstractNodePtr = std::shared_ptr<AbstractNode>;
   using contextPtr = std::shared_ptr<const Context>;
   using ModInst = ModuleInstantiation;

   AbstractModule() : feature(nullptr) {}
   AbstractModule(const Feature& feature) : feature(&feature) {}
   AbstractModule(const Feature *feature) : feature(feature) {}
   virtual ~AbstractModule() {}

   [[nodiscard]] bool is_experimental() const
   {
      return feature != nullptr;
   }

   [[nodiscard]] bool is_enabled() const
   {
      return (feature == nullptr) || feature->is_enabled();
   }

   /**
    * @brief Instantiate the module into the AST.
    * @param defining_context The context where the module was defined.
    * @param inst The instantiation holding args and children if any.
    * @param context context where the module was instantiated.
   **/
   virtual abstractNodePtr instantiate(
      contextPtr const & defining_context,
      ModInst const *inst,
      contextPtr const & context
   ) const = 0;

private:
   const Feature *feature;
};

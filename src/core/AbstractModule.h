#pragma once

#include <memory>

#include <Feature.h>

class AbstractNode;
class Context;
class ModuleInstantiation;

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

   virtual bool is_experimental() const
   {
      return feature != nullptr;
   }
   virtual bool is_enabled() const
   {
      return (feature == nullptr) || feature->is_enabled();
   }
   virtual abstractNodePtr instantiate(
      contextPtr const & defining_context,
      ModInst const *inst,
      contextPtr const & context
   ) const = 0;
private:
   const Feature *feature;
};

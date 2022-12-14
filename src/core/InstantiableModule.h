#pragma once

#include <memory>

class AbstractModule;
class Context;

/**
* @brief An InstantaibleModule stores an AbstractModule with its defining context
**/

struct InstantiableModule
{
  using contextPtr = std::shared_ptr<const Context>;
  contextPtr defining_context;
  AbstractModule const * module;
};

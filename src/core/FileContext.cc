
#include <utils/printutils.h>
#include "Value.h"
#include "SourceFileCache.h"
#include "FileContext.h"
#include "UserModule.h"

boost::optional<CallableFunction>
FileContext::lookup_local_function(const std::string& name, const Location& loc) const
{
  auto result = ScopeContext::lookup_local_function(name, loc);
  if (result) {
    return result;
  }

  for (const auto& m : source_file->usedlibs) {
    // usedmod is nullptr if the library wasn't be compiled (error or file-not-found)
    auto usedmod = SourceFileCache::instance()->lookup(m);
    if (usedmod && usedmod->scope.functions.find(name) != usedmod->scope.functions.end()) {
      ContextHandle<FileContext> context{Context::create<FileContext>(this->parent, usedmod)};
#ifdef DEBUG
      PRINTDB("FileContext for function %s::%s:", m % name);
      PRINTDB("%s", context->dump());
#endif
      return CallableFunction{CallableUserFunction{*context, usedmod->scope.functions[name].get()}};
    }
  }
  return boost::none;
}

boost::optional<InstantiableModule>
FileContext::lookup_local_module(const std::string& name, const Location& loc) const
{
  auto result = ScopeContext::lookup_local_module(name, loc);
  if (result) {
    return result;
  }

  for (const auto& m : source_file->usedlibs) {
    // usedmod is nullptr if the library wasn't be compiled (error or file-not-found)
    auto usedmod = SourceFileCache::instance()->lookup(m);
    if (usedmod && usedmod->scope.modules.find(name) != usedmod->scope.modules.end()) {
      ContextHandle<FileContext> context{Context::create<FileContext>(this->parent, usedmod)};
#ifdef DEBUG
      PRINTDB("FileContext for module %s::%s:", m % name);
      PRINTDB("%s", context->dump());
#endif
      return InstantiableModule{*context, usedmod->scope.modules[name].get()};
    }
  }
  return boost::none;
}

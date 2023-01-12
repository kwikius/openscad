#pragma once

#include "ScopeContext.h"
#include "SourceFile.h"

class FileContext final : public ScopeContext{
public:
  boost::optional<CallableFunction>
    lookup_local_function(const std::string& name, const Location& loc) const override;
  boost::optional<InstantiableModule>
     lookup_local_module(const std::string& name, const Location& loc) const override;

private:
  FileContext(const std::shared_ptr<const Context> parent, const SourceFile *source_file) :
    ScopeContext(parent, &source_file->scope),
    source_file(source_file)
  {}

  const SourceFile *source_file;

  friend class Context;
};

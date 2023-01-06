#pragma once

#include <memory>
#include "ContextFrameHandle.h"

/**
 * Local handle to a all context objects. This is used to maintain the
 * dynamic scoping stack using object lifetime.
 * The Context objects can hang around for longer, e.g. in case of closures.
 */
template <typename T>
class ContextHandle : ContextFrameHandle
{
friend class Context;
private:
  ContextHandle(std::shared_ptr<T>&& context) :
    ContextFrameHandle(context.get()),
    context(std::move(context))
  {
    try {
      this->context->init();
    } catch (...) {
      session->contextMemoryManager().addContext(std::move(this->context));
      throw;
    }
  }
public:
  ~ContextHandle()
  {
    assert(!!session == !!context);
    if (session) {
       session->contextMemoryManager().addContext(std::move(this->context));
    }
  }

  ContextHandle(const ContextHandle&) = delete;
  ContextHandle& operator=(const ContextHandle&) = delete;
  ContextHandle(ContextHandle&& other) = default;

  // Valid only if $other is on the top of the stack.
  ContextHandle& operator=(ContextHandle&& other)
  {
    assert(session);
    assert(context);
    assert(other.context);
    assert(other.session);

    //session->contextMemoryManager().releaseContext();
    session->contextMemoryManager().addContext(std::move(this->context));
    other.release();
    context = std::move(other.context);
    ContextFrameHandle::operator=(context.get());
    return *this;
  }

  const T *operator->() const { return context.get(); }
  T *operator->() { return context.get(); }
  std::shared_ptr<const T> operator*() const { return context; }

private:
  std::shared_ptr<T> context;
};

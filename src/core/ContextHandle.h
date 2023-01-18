#pragma once

#include <memory>
#include <type_traits>
#include "ContextFrameHandle.h"

/**
 * Local handle to a  Context. This is used to maintain the
 * dynamic scoping stack using object lifetime.
 * The Context objects can hang around for longer, e.g. in case of closures.
 */
class Context;

template <typename C>
  requires std::is_base_of_v<Context,C>
class ContextHandle final : ContextFrameHandle
{
friend class Context;
private:
  ContextHandle(std::shared_ptr<C>&& context) :
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

  const C *operator->() const { return context.get(); }
  C *operator->() { return context.get(); }
  std::shared_ptr<const C> operator*() const { return context; }

private:
  std::shared_ptr<C> context;
};

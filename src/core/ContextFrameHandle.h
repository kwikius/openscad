#pragma once

#include <cstddef>
#include "EvaluationSession.h"
#include "ContextFrame.h"
/**
 * @brief Base class for ContextHandle
 * A ContextFrameHandle stores the index the current  ContextFrame, in current EvaluationSeession
 * and keeps it on the special variable stack for the lifetime of the handle.
 * Only intended for a short life on the Context Stack
 **/
class ContextFrameHandle{
protected:
  /**
  *  @brief Create a handle to a context
  *  The Context is always created using Context<C>::create(args..)
  * except for Parameters which is described as "not a true context"
  **/
  friend class Parameters;
  ContextFrameHandle(ContextFrame *frame)
  :session(frame->session()),
  frame_index(session->push_frame(frame))
  {
  }

  ~ContextFrameHandle()
  {
     release();
  }
public:
  ContextFrameHandle(const ContextFrameHandle&) = delete;
  ContextFrameHandle& operator=(const ContextFrameHandle&) = delete;
  ContextFrameHandle& operator=(ContextFrameHandle&&) = delete;

  ContextFrameHandle(ContextFrameHandle&& other) :
    session(other.session),
    frame_index(other.frame_index)
  {
    other.session = nullptr;
  }

  ContextFrameHandle& operator=(ContextFrame *frame)
  {
    assert(session == frame->session());
    session->replace_frame(frame_index, frame);
    return *this;
  }

  // Valid only if handle is on the top of the stack.
  void release()
  {
    if (session) {
      session->pop_frame(frame_index);
      session = nullptr;
    }
  }

protected:
  EvaluationSession *session;
  size_t frame_index;
};

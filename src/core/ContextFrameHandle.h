#pragma once

#include <cstddef>
#include "EvaluationSession.h"
#include "ContextFrame.h"
/*
 * A ContextFrameHandle stores a reference to a ContextFrame, and keeps it on
 * the special variable stack for the lifetime of the handle.
 */

class ContextFrameHandle
{
public:
  ContextFrameHandle(ContextFrame *frame) :
    session(frame->session())
  {
    frame_index = session->push_frame(frame);
  }
  ~ContextFrameHandle()
  {
    release();
  }

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

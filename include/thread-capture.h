/* -----------------------------------------------------------------------------
Copyright 2017 Google Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
----------------------------------------------------------------------------- */

// Author: Kevin P. Barry [ta0kira@gmail.com] [kevinbarry@google.com]

#ifndef THREAD_CAPTURE_H_
#define THREAD_CAPTURE_H_

namespace capture_thread {

// Base class for objects that are used to share information within a thread.
// This class provides access to an API for making an object visible within the
// current thread until the object goes out of scope.
template <class Type>
class ThreadCapture {
 public:
  class CrossThreads;

  // Allows the current object (if any) to be made available in another thread.
  // See thread-crosser.h for canonical usage.
  class ThreadBridge {
   public:
    inline ThreadBridge() : capture_(GetCurrent()) {}

   private:
    ThreadBridge(const ThreadBridge&) = delete;
    ThreadBridge(ThreadBridge&&) = delete;
    ThreadBridge& operator=(const ThreadBridge&) = delete;
    ThreadBridge& operator=(ThreadBridge&&) = delete;
    void* operator new(std::size_t size) = delete;

    friend class CrossThreads;
    Type* const capture_;
  };

 protected:
  class ScopedCapture;

 public:
  // Provides access to an object from another thread within the current thread.
  // See thread-crosser.h for canonical usage.
  class CrossThreads {
   public:
    explicit inline CrossThreads(const ThreadBridge& bridge)
        : previous_(GetCurrent()) {
      SetCurrent(bridge.capture_);
    }

    explicit inline CrossThreads(const ScopedCapture& capture)
        : previous_(GetCurrent()) {
      SetCurrent(capture.current_);
    }

    inline ~CrossThreads() { SetCurrent(previous_); }

   private:
    CrossThreads(const CrossThreads&) = delete;
    CrossThreads(CrossThreads&&) = delete;
    CrossThreads& operator=(const CrossThreads&) = delete;
    CrossThreads& operator=(CrossThreads&&) = delete;
    void* operator new(std::size_t size) = delete;

    Type* const previous_;
  };

 protected:
  ThreadCapture() = default;
  ~ThreadCapture() = default;

  // Add this as a member when defining Type (as a class), and pass 'this' to
  // the constructor to make an instance of Type available within the thread
  // it's allocated in.
  class ScopedCapture {
   public:
    explicit inline ScopedCapture(Type* capture)
        : previous_(GetCurrent()), current_(capture) {
      SetCurrent(capture);
    }

    inline ~ScopedCapture() { SetCurrent(Previous()); }

    Type* Previous() const { return previous_; }

   private:
    ScopedCapture(const ScopedCapture&) = delete;
    ScopedCapture(ScopedCapture&&) = delete;
    ScopedCapture& operator=(const ScopedCapture&) = delete;
    ScopedCapture& operator=(ScopedCapture&&) = delete;
    void* operator new(std::size_t size) = delete;

    friend class CrossThreads;
    Type* const previous_;
    Type* const current_;
  };

  // Gets the most-recent object from the stack of the current thread.
  static inline Type* GetCurrent() { return current_; }

 private:
  ThreadCapture(const ThreadCapture&) = delete;
  ThreadCapture(ThreadCapture&&) = delete;
  ThreadCapture& operator=(const ThreadCapture&) = delete;
  ThreadCapture& operator=(ThreadCapture&&) = delete;

  static inline void SetCurrent(Type* value) { current_ = value; }

  static thread_local Type* current_;
};

template <class Type>
thread_local Type* ThreadCapture<Type>::current_(nullptr);

}  // namespace capture_thread

#endif  // THREAD_CAPTURE_H_

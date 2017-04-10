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

#ifndef THREAD_CROSSER_H_
#define THREAD_CROSSER_H_

#include <functional>

#include "thread-capture.h"

class ThreadCrosser : public ThreadCapture<ThreadCrosser> {
 public:
  static std::function<void()> WrapCall(std::function<void()> call);

 protected:
  ThreadCrosser() : parent_(GetCurrent()), capture_to_(this) {}
  virtual ~ThreadCrosser() = default;

  virtual std::function<void()> WrapFunction(
      std::function<void()> call) const = 0;

 private:
  ThreadCrosser(const ThreadCrosser&) = delete;
  ThreadCrosser(ThreadCrosser&&) = delete;
  ThreadCrosser& operator =(const ThreadCrosser&) = delete;
  ThreadCrosser& operator =(ThreadCrosser&&) = delete;
  void* operator new(std::size_t size) = delete;

  static std::function<void()> WrapCallRec(
      std::function<void()> call, const ThreadCrosser* current);

  // parent_ must stay before capture_to_.
  ThreadCrosser* const parent_;
  const ScopedCapture capture_to_;
  const ThreadCrosser::ThreadBridge bridge_;
};

template <class Type>
class AutoThreadCrosser : public ThreadCrosser, public ThreadCapture<Type> {
 public:
  AutoThreadCrosser(Type* logger) : capture_to_(logger) {}

 protected:
  std::function<void()> WrapFunction(
      std::function<void()> call) const override {
    if (call) {
      return [this, call] {
        const typename ThreadCapture<Type>::CrossThreads logger(bridge_);
        call();
      };
    } else {
      return call;
    }
  }

 private:
  const typename ThreadCapture<Type>::ScopedCapture capture_to_;
  // bridge_ must stay after capture_to_.
  const typename ThreadCapture<Type>::ThreadBridge bridge_;
};

#endif  // THREAD_CROSSER_H_

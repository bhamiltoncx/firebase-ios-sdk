/*
 * Copyright 2015, 2018 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_UTIL_STATUS_H_
#define FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_UTIL_STATUS_H_

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

#include "Firestore/core/include/firebase/firestore/firestore_errors.h"
#include "Firestore/core/src/firebase/firestore/util/firebase_assert.h"
#include "absl/base/attributes.h"
#include "absl/strings/string_view.h"

namespace firebase {
namespace firestore {
namespace util {

/// Denotes success or failure of a call.
class ABSL_MUST_USE_RESULT Status {
 public:
  /// Create a success status.
  Status() {
  }

  /// \brief Create a status with the specified error code and msg as a
  /// human-readable string containing more detailed information.
  Status(FirestoreErrorCode code, absl::string_view msg);

  /// Copy the specified status.
  Status(const Status& s);
  void operator=(const Status& s);

  static Status OK() {
    return Status();
  }

  /// Returns true iff the status indicates success.
  bool ok() const {
    return (state_ == nullptr);
  }

  FirestoreErrorCode code() const {
    return ok() ? FirestoreErrorCode::Ok : state_->code;
  }

  const std::string& error_message() const {
    return ok() ? empty_string() : state_->msg;
  }

  bool operator==(const Status& x) const;
  bool operator!=(const Status& x) const;

  /// \brief If `ok()`, stores `new_status` into `*this`.  If `!ok()`,
  /// preserves the current status, but may augment with additional
  /// information about `new_status`.
  ///
  /// Convenient way of keeping track of the first error encountered.
  /// Instead of:
  ///   `if (overall_status.ok()) overall_status = new_status`
  /// Use:
  ///   `overall_status.Update(new_status);`
  void Update(const Status& new_status);

  /// \brief Return a string representation of this status suitable for
  /// printing. Returns the string `"OK"` for success.
  std::string ToString() const;

  // Ignores any errors. This method does nothing except potentially suppress
  // complaints from any tools that are checking that errors are not dropped on
  // the floor.
  void IgnoreError() const;

 private:
  static const std::string& empty_string();
  struct State {
    FirestoreErrorCode code;
    std::string msg;
  };
  // OK status has a `NULL` state_.  Otherwise, `state_` points to
  // a `State` structure containing the error code and message(s)
  std::unique_ptr<State> state_;

  void SlowCopyFrom(const State* src);
};

inline Status::Status(const Status& s)
    : state_((s.state_ == nullptr) ? nullptr : new State(*s.state_)) {
}

inline void Status::operator=(const Status& s) {
  // The following condition catches both aliasing (when this == &s),
  // and the common case where both s and *this are ok.
  if (state_ != s.state_) {
    SlowCopyFrom(s.state_.get());
  }
}

inline bool Status::operator==(const Status& x) const {
  return (this->state_ == x.state_) || (ToString() == x.ToString());
}

inline bool Status::operator!=(const Status& x) const {
  return !(*this == x);
}

std::ostream& operator<<(std::ostream& os, const Status& x);

typedef std::function<void(const Status&)> StatusCallback;

extern std::string StatusCheckOpHelperOutOfLine(const Status& v,
                                                const char* msg);

#define STATUS_CHECK_OK(val)               \
  FIREBASE_ASSERT_MESSAGE_WITH_EXPRESSION( \
      val.ok(), val.ok(), StatusCheckOpHelperOutOfLine(val, #val).c_str())

// DEBUG only version of STATUS_CHECK_OK.  Compiler still parses 'val' even in
// opt mode.
#define STATUS_DCHECK_OK(val)                  \
  FIREBASE_DEV_ASSERT_MESSAGE_WITH_EXPRESSION( \
      val.ok(), val.ok(), StatusCheckOpHelperOutOfLine(val, #val).c_str())

}  // namespace util
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_UTIL_STATUS_H_

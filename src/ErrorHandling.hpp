/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include <sstream>
#include <functional>
#include "location.hh"

namespace OpenABL {

struct Error {
  Error(std::string msg, location loc)
    : msg{msg}, loc{loc} {}

  std::string msg;
  location loc;
};

struct ErrorStream {
  typedef std::function<void (const Error &)> HandlerType;

  ErrorStream(HandlerType handler)
    : handler{handler} {}

  template<typename T>
  ErrorStream &operator<<(T&& a) {
    curMsg << a;
    return *this;
  }

  ErrorStream &operator<<(location loc) {
    curLoc = loc;
    finishError();
    return *this;
  }

private:
  void finishError() {
    Error err { curMsg.str(), curLoc };
    curMsg.str("");
    handler(err);
  }

  HandlerType handler;
  std::stringstream curMsg;
  location curLoc;
};
}

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

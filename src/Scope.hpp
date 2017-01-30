#pragma once

#include <map>
#include <cassert>

namespace OpenABL {

// Globally unique variable id, to distinguish
// different variables that have the same name.
struct VarId {
  static VarId make() {
    return VarId { ++max_id };
  }

  bool operator<(const VarId &other) const {
    return id < other.id;
  }

  VarId() : id{0} {}

private:
  VarId(uint32_t id) : id{id} {}

  static uint32_t max_id;
  uint32_t id;
};

struct ScopeEntry {
};

struct Scope {
  void add(VarId var) {
    vars.insert({ var, ScopeEntry {} });
  }

private:
  std::map<VarId, ScopeEntry> vars;
};

}

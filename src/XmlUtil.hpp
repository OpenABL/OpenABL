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
#include <vector>

namespace OpenABL {

// Not terribly efficient, but very convenient representation
struct XmlElem {
  typedef std::pair<std::string, std::string> Attr;

  XmlElem(const std::string &text)
    : text(text) {}
  XmlElem(const std::string &name, std::vector<XmlElem> children)
    : name(name), children(std::move(children)) {}

  void setAttr(const std::string &name, const std::string &value) {
    attrs.push_back({ name, value });
  }

private:
  std::string name;
  std::string text;
  std::vector<XmlElem> children;
  std::vector<Attr> attrs;

  friend struct XmlWriter;
};

using XmlElems = std::vector<XmlElem>;

// IMPORTANT: This doesn't implement any escaping, as it's currently
// not necessary. Should be added once it's needed.
struct XmlWriter {
  XmlWriter() : indentLevel{0} {}

  std::string serialize(const XmlElem &root) {
    s << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    write(root);

    std::string result = s.str();
    s.str("");
    s.clear();
    return result;
  }

private:
  void nl() { s << "\n" << std::string(2 * indentLevel, ' '); }
  void indent() { indentLevel++; }
  void outdent() { indentLevel--; }

  void write(const XmlElem &elem) {
    if (elem.name.empty()) {
      s << elem.text;
      return;
    }

    s << "<" << elem.name;
    for (const XmlElem::Attr &attr : elem.attrs) {
      s << " " << attr.first << "=\"" << attr.second << "\"";
    }

    if (elem.children.empty()) {
      s << " />";
      return;
    }

    s << ">";
    if (elem.children.size() == 1 && elem.children[0].name.empty()) {
      s << elem.children[0].text;
    } else {
      indent();
      for (const XmlElem &child : elem.children) {
        nl();
        write(child);
      }
      outdent();
      nl();
    }
    s << "</" << elem.name << ">";
  }

  std::ostringstream s;
  unsigned indentLevel;
};

}

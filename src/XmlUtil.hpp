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

//
// Created by bertrand on 25/09/2021.
//

#include "ContentTable.h"
#include "../Section.h"

#include <fmt/format.h>

#include <sstream>

void ContentTable::Build(std::string &str, const std::vector<Section>& sections) {
  std::stringstream ss;
  ss << R"(<div class="tableOfContent">)";
  for (const auto &section: sections) {
    ss << fmt::format(R"(<a href="#{}" class="sectionTitle">{}</a>)", section.title, section.title);
    for (const auto &doc: section.items) {
      ss << fmt::format(R"(<a href="#{}" class="docTitle">{}</a>)", doc.title, doc.title);
    }
  }
  ss << R"(</div>)";
  str.append(ss.str());
}

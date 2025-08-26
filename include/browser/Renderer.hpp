#pragma once
#include "HtmlParser.hpp"
#include <string>

namespace render {

// Print a parsed page in a readable terminal format
void to_console(const std::string& url, const ParsedPage& page);

} // namespace render

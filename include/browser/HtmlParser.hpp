#pragma once
#include <string>
#include <vector>

struct Link {
    std::string text;
    std::string href;
};

struct ParsedPage {
    std::string title;
    std::vector<std::string> h1s;
    std::vector<Link> links;
    std::string plain_text; // very basic
};

namespace html {

// Parse HTML (uses Gumbo if available, else a simple fallback)
ParsedPage parse(const std::string& html);

} // namespace html

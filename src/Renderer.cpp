#include "browser/Renderer.hpp"
#include <iostream>

namespace render {

void to_console(const std::string& url, const ParsedPage& p) {
    std::cout << "\n==================== PAGE ====================\n";
    std::cout << "URL: " << url << "\n";
    std::cout << "Title: " << (p.title.empty() ? "(none)" : p.title) << "\n\n";

    if (!p.h1s.empty()) {
        std::cout << "# Headings (H1)\n";
        for (auto& h : p.h1s) std::cout << "  - " << h << "\n";
        std::cout << "\n";
    }

    std::cout << "Links:\n";
    int idx = 1;
    for (auto& L : p.links) {
        std::cout << "  [" << idx++ << "] " << (L.text.empty() ? L.href : L.text) << " -> " << L.href << "\n";
    }
    if (p.links.empty()) std::cout << "  (none)\n";

    std::cout << "\n--- Snippet ---\n";
    if (p.plain_text.size() > 400) std::cout << p.plain_text.substr(0, 400) << "...\n";
    else std::cout << p.plain_text << "\n";
    std::cout << "==============================================\n";
}

} // namespace render

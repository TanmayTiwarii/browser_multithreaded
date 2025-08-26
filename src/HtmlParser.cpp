#include "browser/HtmlParser.hpp"
#include <algorithm>
#include <cctype>
#include <regex>
#include <string>

#ifdef USE_GUMBO
  #include <gumbo.h>
#endif

namespace {

// super small helper: strip HTML tags to plain-ish text (fallback)
std::string strip_tags(const std::string& html) {
    std::string s = html;
    // Remove script/style blocks crudely
    s = std::regex_replace(s, std::regex(R"(<script[\s\S]*?</script>)", std::regex::icase), "");
    s = std::regex_replace(s, std::regex(R"(<style[\s\S]*?</style>)", std::regex::icase), "");
    // Remove tags
    s = std::regex_replace(s, std::regex(R"(<[^>]+>)"), "");
    // Collapse spaces
    s = std::regex_replace(s, std::regex(R"(\s+)"), " ");
    return s;
}

#ifndef USE_GUMBO
// naive extractors for <title>, <h1>, <a>
std::string extract_first_between(const std::string& s, const std::string& open, const std::string& close) {
    auto p = std::search(
        s.begin(), s.end(), open.begin(), open.end(),
        [](char a, char b){ return std::tolower(a) == std::tolower(b); }
    );
    if (p == s.end()) return "";
    auto start = p + open.size();
    auto q = std::search(
        start, s.end(), close.begin(), close.end(),
        [](char a, char b){ return std::tolower(a) == std::tolower(b); }
    );
    if (q == s.end()) return "";
    return std::string(start, q);
}

std::vector<std::string> extract_all_between(const std::string& s, const std::string& open, const std::string& close) {
    std::vector<std::string> out;
    auto it = s.begin();
    while (true) {
        auto p = std::search(it, s.end(), open.begin(), open.end(),
                             [](char a, char b){ return std::tolower(a) == std::tolower(b); });
        if (p == s.end()) break;
        auto start = p + open.size();
        auto q = std::search(start, s.end(), close.begin(), close.end(),
                             [](char a, char b){ return std::tolower(a) == std::tolower(b); });
        if (q == s.end()) break;
        out.emplace_back(start, q);
        it = q + close.size();
    }
    return out;
}

std::vector<Link> extract_links(const std::string& s) {
    std::vector<Link> links;
    // Very loose regex for <a href="...">text</a>
    std::regex re(R"(<a[^>]*href\s*=\s*["']([^"']+)["'][^>]*>(.*?)</a>)", std::regex::icase);
    auto begin = std::sregex_iterator(s.begin(), s.end(), re);
    auto end   = std::sregex_iterator();
    for (auto i = begin; i != end; ++i) {
        Link L;
        L.href = (*i)[1].str();
        L.text = strip_tags((*i)[2].str());
        links.push_back(L);
    }
    return links;
}
#endif

} // namespace

namespace html {

ParsedPage parse(const std::string& html) {
    ParsedPage out;

#ifdef USE_GUMBO
    GumboOutput* g = gumbo_parse(html.c_str());
    // title
    {
        // Find <title>
        const GumboVector* root_children = &g->root->v.element.children;
        // Very small DFS
        std::vector<GumboNode*> stack;
        for (unsigned i = 0; i < root_children->length; ++i)
            stack.push_back(static_cast<GumboNode*>(root_children->data[i]));

        while (!stack.empty()) {
            GumboNode* n = stack.back(); stack.pop_back();
            if (n->type == GUMBO_NODE_ELEMENT) {
                if (n->v.element.tag == GUMBO_TAG_TITLE && n->v.element.children.length > 0) {
                    GumboNode* t = static_cast<GumboNode*>(n->v.element.children.data[0]);
                    if (t && t->type == GUMBO_NODE_TEXT) out.title = t->v.text.text;
                }
                // collect H1 and links
                if (n->v.element.tag == GUMBO_TAG_H1) {
                    if (n->v.element.children.length > 0) {
                        GumboNode* t = static_cast<GumboNode*>(n->v.element.children.data[0]);
                        if (t && t->type == GUMBO_NODE_TEXT) out.h1s.emplace_back(t->v.text.text);
                    }
                }
                if (n->v.element.tag == GUMBO_TAG_A) {
                    GumboAttribute* href = gumbo_get_attribute(&n->v.element.attributes, "href");
                    std::string link_text;
                    if (n->v.element.children.length > 0) {
                        GumboNode* t = static_cast<GumboNode*>(n->v.element.children.data[0]);
                        if (t) {
                            if (t->type == GUMBO_NODE_TEXT) link_text = t->v.text.text;
                        }
                    }
                    if (href) out.links.push_back({link_text, href->value});
                }
                // push children
                for (unsigned i = 0; i < n->v.element.children.length; ++i) {
                    stack.push_back(static_cast<GumboNode*>(n->v.element.children.data[i]));
                }
            }
        }
    }
    out.plain_text = strip_tags(html);
    gumbo_destroy_output(g);
#else
    // Fallback: naive extraction
    out.title = extract_first_between(html, "<title>", "</title>");
    out.h1s   = extract_all_between(html, "<h1>", "</h1>");
    out.links = extract_links(html);
    out.plain_text = strip_tags(html);
#endif

    return out;
}

} // namespace html

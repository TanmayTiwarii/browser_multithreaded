#include "browser/Browser.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::vector<std::string> urls;

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) urls.emplace_back(argv[i]);
    } else {
        // Some HTTP (not HTTPS) pages you can try:
        urls = {
            "http://example.com/",
            "http://info.cern.ch/",
            "http://neverssl.com/"
        };
    }

    std::cout << "MiniBrowser: fetching " << urls.size()
              << " page(s) concurrently...\n";

    Browser b;
    auto results = b.open_many(urls);

    std::cout << "\nDone. " << results.size() << " tab(s) processed.\n";
    return 0;
}

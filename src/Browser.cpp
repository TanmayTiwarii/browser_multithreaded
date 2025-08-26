#include "browser/Browser.hpp"
#include "browser/HttpClient.hpp"
#include "browser/HtmlParser.hpp"
#include "browser/Renderer.hpp"
#include <iostream>

std::vector<TabResult> Browser::open_many(const std::vector<std::string>& urls) {
    std::vector<std::future<TabResult>> futs;
    futs.reserve(urls.size());

    for (auto& u : urls) {
        futs.emplace_back(std::async(std::launch::async, [this, u]() -> TabResult {
            TabResult tr;
            tr.url = u;
            try {
                auto resp = http::get(u, 10000, 5);
                if (resp.status_code >= 200 && resp.status_code < 300) {
                    tr.page = html::parse(resp.body);
                    tr.ok = true;
                } else {
                    tr.error = "HTTP " + std::to_string(resp.status_code) + " " + resp.status_message;
                }
            } catch (const std::exception& e) {
                tr.error = e.what();
            }

            {
                std::lock_guard<std::mutex> lock(cout_mtx_);
                if (tr.ok) render::to_console(u, tr.page);
                else std::cerr << "[ERROR] " << u << " -> " << tr.error << "\n";
            }
            return tr;
        }));
    }

    std::vector<TabResult> results;
    results.reserve(urls.size());
    for (auto& f : futs) results.push_back(f.get());
    return results;
}

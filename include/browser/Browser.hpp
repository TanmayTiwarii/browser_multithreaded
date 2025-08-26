#pragma once
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include "HtmlParser.hpp"

struct TabResult {
    std::string url;
    ParsedPage  page;
    bool        ok = false;
    std::string error;
};

class Browser {
public:
    // Launches concurrent fetch+parse for each URL
    std::vector<TabResult> open_many(const std::vector<std::string>& urls);

private:
    std::mutex cout_mtx_;
};

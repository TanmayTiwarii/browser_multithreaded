#pragma once
#include <string>
#include <map>
#include <vector>

struct HttpResponse {
    std::string http_version;
    int         status_code = 0;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct UrlParts {
    std::string scheme;  // "http"
    std::string host;
    std::string port;    // "80"
    std::string path;    // "/..."
};

namespace http {

// Parse a URL into parts (supports only http)
bool parse_url(const std::string& url, UrlParts& out);

// Blocking GET with redirects (max_redirects)
HttpResponse get(const std::string& url, int timeout_ms = 10000, int max_redirects = 5);

// Parse raw HTTP message to HttpResponse
HttpResponse parse_http_response(const std::string& raw);

// Utility (lower-cases header key)
std::string to_lower(std::string s);

} // namespace http

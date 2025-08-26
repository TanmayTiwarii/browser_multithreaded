#define _WIN32_WINNT 0x0600
#include "browser/HttpClient.hpp"
#include <sstream>
#include <cstring>
#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <unistd.h>
  #define closesocket close
#endif

namespace {

std::string read_all(int sock, int timeout_ms) {
    // Simple blocking read-until-close. (You can set timeouts if desired.)
#ifdef _WIN32
    // Windows: set recv timeout
    int to = timeout_ms;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&to, sizeof(to));
#else
    timeval tv{};
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    std::string out;
    char buf[8192];
    for (;;) {
        int n = recv(sock, buf, sizeof(buf), 0);
        if (n > 0) out.append(buf, buf + n);
        else break; // 0 or error -> done
    }
    return out;
}

std::string dechunk(const std::string& body) {
    // Very small chunked decoder (no extensions)
    std::istringstream in(body);
    std::string line;
    std::string out;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // line is chunk size in hex
        std::istringstream hexs(line);
        size_t sz = 0;
        hexs >> std::hex >> sz;
        if (sz == 0) break;

        std::string chunk(sz, '\0');
        in.read(&chunk[0], sz);
        out += chunk;

        // consume trailing \r\n after chunk
        std::getline(in, line);
    }
    return out;
}

} // namespace

namespace http {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

bool parse_url(const std::string& url, UrlParts& out) {
    // supports only http://host[:port]/path
    const std::string http = "http://";
    if (url.rfind(http, 0) != 0) return false;

    out.scheme = "http";
    std::string rest = url.substr(http.size());

    auto slash = rest.find('/');
    std::string hostport = (slash == std::string::npos) ? rest : rest.substr(0, slash);
    out.path = (slash == std::string::npos) ? "/" : rest.substr(slash);

    auto colon = hostport.find(':');
    if (colon == std::string::npos) {
        out.host = hostport;
        out.port = "80";
    } else {
        out.host = hostport.substr(0, colon);
        out.port = hostport.substr(colon + 1);
        if (out.port.empty()) out.port = "80";
    }
    return !out.host.empty();
}

HttpResponse parse_http_response(const std::string& raw) {
    HttpResponse r;
    std::istringstream s(raw);
    std::string line;

    // status line
    std::getline(s, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    {
        std::istringstream ss(line);
        ss >> r.http_version >> r.status_code;
        std::getline(ss, r.status_message);
        if (!r.status_message.empty() && r.status_message.front() == ' ')
            r.status_message.erase(0, 1);
    }

    // headers
    while (std::getline(s, line)) {
        if (line == "\r" || line == "") break;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string k = line.substr(0, pos);
            std::string v = line.substr(pos + 1);
            if (!v.empty() && v.front() == ' ') v.erase(0, 1);
            r.headers[to_lower(k)] = v;
        }
    }

    // body (rest)
    std::string body((std::istreambuf_iterator<char>(s)),
                      std::istreambuf_iterator<char>());

    // handle chunked
    auto it = r.headers.find("transfer-encoding");
    if (it != r.headers.end() && to_lower(it->second).find("chunked") != std::string::npos) {
        r.body = dechunk(body);
    } else {
        r.body = body;
    }
    return r;
}

HttpResponse get(const std::string& url, int timeout_ms, int max_redirects) {
#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);
#endif

    UrlParts u;
    if (!parse_url(url, u)) {
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("Only http:// URLs are supported. Bad URL: " + url);
    }

    HttpResponse final_resp;

    std::string current = url;
    for (int redirect = 0; redirect <= max_redirects; ++redirect) {

        parse_url(current, u);

        addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        int rc = getaddrinfo(u.host.c_str(), u.port.c_str(), &hints, &res);
        if (rc != 0) {
#ifdef _WIN32
            WSACleanup();
#endif
            throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerrorA(rc));
        }

        int sock = -1;
        for (addrinfo* p = res; p; p = p->ai_next) {
            sock = (int)socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sock < 0) continue;
            if (connect(sock, p->ai_addr, (int)p->ai_addrlen) == 0) {
                break;
            }
            closesocket(sock);
            sock = -1;
        }
        freeaddrinfo(res);
        if (sock < 0) {
#ifdef _WIN32
            WSACleanup();
#endif
            throw std::runtime_error("Connection failed to " + u.host + ":" + u.port);
        }

        std::ostringstream req;
        req << "GET " << u.path << " HTTP/1.1\r\n"
            << "Host: " << u.host << "\r\n"
            << "Connection: close\r\n"
            << "User-Agent: MiniBrowser/0.1\r\n"
            << "Accept: */*\r\n\r\n";

        const auto str = req.str();
        send(sock, str.c_str(), (int)str.size(), 0);

        std::string raw = read_all(sock, timeout_ms);
        closesocket(sock);

        auto resp = parse_http_response(raw);

        if (resp.status_code == 301 || resp.status_code == 302 || resp.status_code == 307 || resp.status_code == 308) {
            auto loc = resp.headers.find("location");
            if (loc == resp.headers.end()) {
                final_resp = resp;
                break;
            }
            // Only support absolute http:// or host-relative
            std::string L = loc->second;
            if (L.rfind("http://", 0) == 0) {
                current = L;
            } else if (!L.empty() && L[0] == '/') {
                current = "http://" + u.host + L;
            } else {
                // relative without slash -> simple join
                if (u.path.back() != '/') {
                    auto slash = u.path.find_last_of('/');
                    std::string base = (slash == std::string::npos) ? "/" : u.path.substr(0, slash + 1);
                    current = "http://" + u.host + base + L;
                } else {
                    current = "http://" + u.host + u.path + L;
                }
            }
            continue; // follow redirect
        }

        final_resp = resp;
        break;
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return final_resp;
}

} // namespace http

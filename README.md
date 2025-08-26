# 🖥️ Multithreaded C++ Mini Browser

A lightweight **multithreaded web browser** built from scratch in **C++**.  
It fetches multiple web pages concurrently, parses their HTML, and displays key information (titles, headings, and links) in the terminal.

---

## 🚀 Features
- 🌐 **HTTP client** implemented using raw sockets (no external HTTP libraries).
- ⚡ **Multithreading support** – fetch multiple pages (tabs) in parallel.
- 📑 **HTML parsing** using [Gumbo Parser](https://github.com/google/gumbo-parser).
- 🔍 Extracts:
  - Page title
  - `<h1>` headings
  - Hyperlinks (`<a href>`)
- 🛠️ **CMake-based build system** for portability.
- ✅ Works on **Windows (MinGW)** and **Linux**.

---

## 🛠️ Tech Stack
- **Languages**: C++17
- **Build System**: CMake
- **Libraries**:
  - Winsock2 / POSIX Sockets (Networking)
  - Gumbo Parser (HTML parsing)
  - `std::thread`, `std::mutex` (Concurrency)

---

## 📦 Installation

### 1. Clone the repository
```bash
git clone https://github.com/<your-username>/browser_multithreaded.git
cd browser_multithreaded
```
### 2. Build the project
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

For Linux:
```bash
mkdir build && cd build
cmake ..
make
```
## ▶️ Usage

### Run the browser with one or more URLs:
```bash
./browser http://example.com http://neverssl.com
```

### Sample Output:
```bash
MiniBrowser: fetching 2 page(s) concurrently...

==================== PAGE ====================
URL: http://example.com
Title: Example Domain

# Headings (H1)
  - Example Domain

Links:
  [1] More information... -> https://www.iana.org/domains/example

--- Snippet ---
 Example Domain Example Domain This domain is for use in illustrative examples...
==============================================

[ERROR] http://neverssl.com -> Connection failed
```

## 📚 Learning Outcomes

- Through this project, I learned:

- How low-level networking works (TCP/IP, sockets, ports).

- The HTTP request/response lifecycle.

- Parsing HTML with Gumbo.

- Multithreading and synchronization in C++.

- Using CMake for cross-platform builds.

## 📌 Roadmap

 - Add HTTPS (TLS/SSL) support.

 - Basic GUI for tab management.

 - Support for images and CSS parsing.

 - Improved error handling & caching.

## 🤝 Contributing

Pull requests are welcome! For major changes, please open an issue first to discuss.

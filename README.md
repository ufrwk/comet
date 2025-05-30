# Comet

[中文说明](README_zh.md)

**Comet** is a minimal C++11 component host system for lightweight runtime modularity.  
It allows objects to register multiple interfaces into a local container and query each other without global context.

### When to Use
Comet is ideal for games, tools, or modular services where component relationships are local, static, and don't require heavy DI frameworks.

### Key Features

- No 3rd dependencies
- No macros, no RTTI abuse
- No base class pollution in business interfaces
- Single-process, single-module design
- Safe interface-to-interface casting via `CComPtr<>`

### How It Works

- Inherit your interfaces from `IComUnknown`
- Implement them using `CComUnknown<YourInterface>`
- Inject components into a host (`IComHost`)
- Use `CComPtr<T>` to access other components within the same host

### Example

```cpp
#include <comet.hpp>
#include <iostream>

struct ILogger1 : IComUnknown {
    virtual void Log(const std::string&) = 0;
};

struct ILogger2 : IComUnknown {
    virtual void Log(const std::string&) = 0;
};

class CLogger1 : public CComUnknown<ILogger1> {
public:
    void Log(const std::string& msg) override {
        std::cout << "[Logger1] " << msg << "\n";
    }
};

class CLogger2 : public CComUnknown<ILogger2> {
public:
    void Log(const std::string& msg) override {
        std::cout << "[Logger2] " << msg << "\n";

        // internal cross-module access (same host)
        CComPtr<ILogger1> log1(this);
        log1->Log("Internal call from Logger2 to Logger1");
    }
};

int main() {
    IComHost* host = NewComHost();

    // Install components
    host->Attach<ILogger1>(new CLogger1());
    host->Attach<ILogger2>(new CLogger2());

    // Access one component
    CComPtr<ILogger1> logger1(host);
    logger1->Log("Hello from Logger1");

    // Cross-interface cast (within same host)
    CComPtr<ILogger2> logger2(logger1);
    logger2->Log("Hello from Logger2");

    // Uninstall components
    delete static_cast<CLogger1*>(host->Detach<ILogger1>());
    delete static_cast<CLogger2*>(host->Detach<ILogger2>());
    host->Deref();

    return 0;
}
```

### Requirements

* C++11 or newer
* Single-threaded usage (no locking provided)

### License

This project is licensed under the MIT License.

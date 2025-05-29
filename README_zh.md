# Comet

Comet 是一个基于 C++11 的轻量级组件宿主系统, 专为运行时模块化设计.   
它允许多个接口对象注册到本地容器中, 并在不依赖全局上下文的前提下相互访问. 

---

## 适用场景

Comet 特别适用于以下类型的项目：

- 游戏逻辑组件系统  
- 工具链中模块解耦  
- 微服务或插件框架中的轻量运行时组件组织  

特别适用于组件之间关系局部/静态/明确, 并不需要重型依赖注入(DI)框架的项目. 

---

## 核心特性

- 无第三方依赖  
- 无宏魔法/无 RTTI 滥用  
- 业务接口不污染, 无需继承特定基类  
- 设计上支持单进程/单模块使用(也可跨模块扩展)  
- 使用 `CComPtr<>` 实现类型安全的接口间访问  

---

## 使用方式

### 步骤概览

1. 接口继承自 `IComUnknown`  
2. 实现类继承自 `CComUnknown<YourInterface>`  
3. 将实现注册进组件宿主 `IComHost`  
4. 使用 `CComPtr<T>` 查询和访问其它组件  

---

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

        // 内部跨接口访问 (同 host)
        CComPtr<ILogger1> log1(this);
        log1->Log("Internal call from Logger2 to Logger1");
    }
};

int main() {
    IComHost* host = NewComHost();

    // 初始化组件集合
    host->Attach<ILogger1>(new CLogger1());
    host->Attach<ILogger2>(new CLogger2());

    // 通过 host 获得组件
    CComPtr<ILogger1> logger1(host);
    logger1->Log("Hello from Logger1");

    // 通过组件获得组件 (同 host)
    CComPtr<ILogger2> logger2(logger1);
    logger2->Log("Hello from Logger2");

    // 清理 & 卸载
    delete static_cast<CLogger1*>(host->Detach<ILogger1>());
    delete static_cast<CLogger2*>(host->Detach<ILogger2>());
    host->Delete();

    return 0;
}
```

---

## 编译要求

- C++11 或更新版本  
- 单线程使用(无线程安全机制)  

---

## 许可证

本项目采用 MIT License 开源协议. 

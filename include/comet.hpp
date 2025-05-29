#pragma once

struct IComHost;
// Creates a new standalone component container (host) with optional slot hint
IComHost* NewComHost(unsigned int hint = 64);

namespace std {
class type_info;
};

namespace comet {
template <class T>
struct type_ident {
    static constexpr const void* get() noexcept {
        return &typeid(T);
    }
};

template <class T>
constexpr const void* ident() noexcept {
    return type_ident<T>::get();
}
}; // namespace comet

// Common base interface for components that support host-based interface lookup
struct IComUnknown {
    template <class>
    friend class CComPtr;
    template <class>
    friend class CComUnknown;

private:
    virtual void* __QueryInterface(const void* type) = 0;
};

// Component container that manages registration and cross-interface lookup
struct IComHost : IComUnknown {
public:
    virtual void Delete() noexcept = 0;

public:
    // Attaches a component that supports cross-component access (host-aware)
    template <class Interface, class Implement>
    Interface* Attach(Implement* impl) {
        impl->__SetComHost(this);
        return RawAttach<Interface, Implement>(impl);
    }
    // Attaches a passive component that does not access others
    template <class Interface, class Implement>
    Interface* RawAttach(Implement* impl) {
        Interface* intfptr = impl; // Implement is derived from Interface
        void* ptr = this->__Attach(comet::ident<Interface>(), intfptr);
        return reinterpret_cast<Interface*>(ptr);
    }

    // Detaches and returns the component previously registered under the given interface
    template <class Interface>
    Interface* Detach() {
        void* impl = this->__Detach(comet::ident<Interface>());
        return reinterpret_cast<Interface*>(impl);
    }

private:
    virtual void* __Detach(const void* type) = 0;
    virtual void* __Attach(const void* type, void* impl) = 0;
};

// Default implementation for IComUnknown enabling cross-component queries via host
template <class T>
class CComUnknown : public T {
    friend struct IComHost;

private:
    void __SetComHost(IComHost* host) noexcept {
        m_host = host;
    }
    void* __QueryInterface(const void* type) override final {
        if (auto* host = m_host) {
            return host->__QueryInterface(type);
        }
        return nullptr;
    }

private:
    IComHost* m_host{};
};

// A lightweight wrapper for interface-based component lookup
template <class T>
class CComPtr final {
public:
    template <class Other>
    CComPtr(const CComPtr<Other>& ptr) : CComPtr(ptr.get()) {
    }
    CComPtr(IComUnknown* ptr) : m_impl(reinterpret_cast<T*>(ptr->__QueryInterface(comet::ident<T>()))) {
    }

    T* get() const noexcept {
        return m_impl;
    }
    T* operator->() const noexcept {
        return m_impl;
    }
    explicit operator bool() const noexcept {
        return (m_impl != nullptr);
    }

private:
    T* const m_impl;
};

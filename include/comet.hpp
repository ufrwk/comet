#pragma once
#include <typeindex>

struct IComHost;
IComHost* NewComHost();

struct IComUnknown {
    template <class>
    friend class CComPtr;
    template <class>
    friend class CComUnknown;

private:
    virtual void* __QueryInterface(const std::type_index& type) = 0;
};

struct IComHost : IComUnknown {
public:
    virtual void Delete() = 0;

public:
    template <class Interface, class Implement>
    Interface* Inject(Implement* impl) {
        impl->__SetComHost(this);
        void* ptr = this->__Inject(std::type_index(typeid(Interface)), impl);
        return reinterpret_cast<Interface*>(ptr);
    }

    template <class Interface>
    Interface* Enject() {
        void* impl = this->__Enject(std::type_index(typeid(Interface)));
        return reinterpret_cast<Interface*>(impl);
    }

private:
    virtual void* __Enject(const std::type_index& type) = 0;
    virtual void* __Inject(const std::type_index& type, void* impl) = 0;
};

template <class T>
class CComUnknown : public T {
    friend struct IComHost;

private:
    inline void __SetComHost(IComHost* host) {
        m_host = host;
    }
    void* __QueryInterface(const std::type_index& type) override final {
        if (auto* host = m_host) {
            return host->__QueryInterface(type);
        }
        return nullptr;
    }

private:
    IComHost* m_host{};
};

template <class T>
class CComPtr final {
public:
    template <class Other>
    CComPtr(const CComPtr<Other>& ptr) : CComPtr(ptr.get()) {
    }
    CComPtr(IComUnknown* ptr) : m_impl(reinterpret_cast<T*>(ptr->__QueryInterface(std::type_index(typeid(T))))) {
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

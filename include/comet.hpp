#pragma once
#include <typeinfo>
#include "comet/traits.hpp"

struct IComHost;
// Creates a new standalone component container (host) with optional slot hint
IComHost* NewComHost(unsigned int hint = 64);

struct IComDeleter {
    virtual void OnDelete() = 0;
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

template <class T>
class CComDeleter final : public IComDeleter {
public:
    void OnDelete() override {
        m_func();
        delete this;
    }

    static IComDeleter* Create(T&& func) {
        return new CComDeleter<T>(forward<T>(func));
    }

protected:
    CComDeleter(T&& func) : m_func(forward<T>(func)) {
    }

private:
    typename decay<T>::type m_func;
};

}; // namespace comet

/**
 * @brief Define a stable type identity for use across module boundaries.
 *
 * This macro specializes `comet::type_ident<T>` with a fixed ID.
 *
 * Use it when `&typeid(T)` is not reliable (e.g., across DLL/plugin boundaries).
 *
 * @param Type The interface type to identify.
 * @param ID   A non-zero constant castable to `const void*`.
 *
 * Example:
 * @code
 * COMET_DEFINE_TYPE_IDENT(IFoo, 123);
 * @endcode
 */
#define COMET_DEFINE_TYPE_IDENT(Type, ID)             \
    static_assert((ID) != 0, "ID must be non-zero");  \
    namespace comet {                                 \
    template <>                                       \
    struct type_ident<Type> {                         \
        static constexpr const void* get() noexcept { \
            return reinterpret_cast<const void*>(ID); \
        }                                             \
    };                                                \
    }

// Common base interface for components that support host-based interface lookup
struct IComUnknown {
    friend class CComHost;
    template <class>
    friend class CComPtr;
    template <class>
    friend class CComUnknown;

private:
    virtual void* __QueryInterface(const void* type) = 0;
};

// Component container that manages registration and cross-interface lookup
struct IComHost : IComUnknown {
    friend class CComHost;

public:
    virtual void Ref() noexcept = 0;
    virtual void Deref() noexcept = 0;

    // LIFO, like `atexit`
    virtual void AtDelete(IComDeleter* deleter) = 0;

    // upward lookup
    virtual bool AddChild(IComHost* child) = 0;
    virtual bool RemoveChild(IComHost* child) = 0;
    virtual IComHost* GetParent() const noexcept = 0;

private:
    virtual void* __Detach(const void* type) = 0;
    virtual void* __Attach(const void* type, void* impl) = 0;
    virtual bool __SetParent(IComHost* parent) = 0;

public:
    // Attaches a component that supports cross-component access (host-aware)
    template <class Interface, class Implement>
    Interface* Attach(Implement* impl) {
        impl->__SetComHost(this);
        return this->RawAttach<Interface, Implement>(impl);
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

    template <class T>
    void DeferDelete(T&& func) {
        this->AtDelete(comet::CComDeleter<T>::Create(comet::forward<T>(func)));
    }

    template <class Interface, class Implement, class T>
    Interface* AttachWithDeleter(Implement* impl, T&& func) {
        Interface* intfptr = this->Attach<Interface, Implement>(impl);
        if (!intfptr) {
            return nullptr; // failed
        }

        struct Callable {
            Implement* impl;
            typename comet::decay<T>::type func;
            void operator()() {
                func(impl);
            }
        };

        this->DeferDelete(Callable{impl, comet::forward<T>(func)});
        return intfptr;
    }

    template <class Interface, class Implement, class T>
    Interface* RawAttachWithDeleter(Implement* impl, T&& func) {
        Interface* intfptr = this->RawAttach<Interface, Implement>(impl);
        if (!intfptr) {
            return nullptr; // failed
        }

        struct Callable {
            Implement* impl;
            typename comet::decay<T>::type func;
            void operator()() {
                func(impl);
            }
        };

        this->DeferDelete(Callable{impl, comet::forward<T>(func)});
        return intfptr;
    }
};

// Default implementation for IComUnknown enabling cross-component queries via host
template <class T>
class CComUnknown : public T {
    friend struct IComHost;

private:
    void __SetComHost(IComHost* host) noexcept {
        m_com_host = host;
    }
    void* __QueryInterface(const void* type) override final {
        if (auto* host = m_com_host) {
            return host->__QueryInterface(type);
        }
        return nullptr;
    }

private:
    IComHost* m_com_host{};
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

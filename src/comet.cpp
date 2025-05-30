#include "comet.hpp"

#include <vector>
#include <unordered_map>

#include <cassert>
#include <algorithm>

class CComHost final : public IComHost {
public:
    CComHost(unsigned int hint) {
        if (hint > 0) {
            m_impl_table.reserve(hint);
        }
    }

    ~CComHost() {
        for (IComHost* child : m_children) {
            child->Deref();
        }
    }

public:
    void Ref() noexcept override {
        m_refc++;
    }

    void Deref() noexcept override {
        if (--m_refc == 0) {
            delete this;
        }
    }

    bool AddChild(IComHost* child) override {
        if (this == child) {
            assert(false && "add self as child");
            return false;
        }
        for (IComHost* pos = this; pos; pos = pos->GetParent()) {
            if (pos == child) {
                assert(false && "cyclic reference");
                return false;
            }
        }
        if (!child->__SetParent(this)) {
            return false;
        }

        child->Ref();
        m_children.push_back(child);
        return true;
    }

    bool RemoveChild(IComHost* child) override {
        if (this != child->GetParent()) {
            return false;
        }
        auto i = std::find(m_children.begin(), m_children.end(), child);
        if (i == m_children.end()) {
            assert(false && "invalid child relationship");
            return false;
        }
        child->__SetParent(nullptr);
        m_children.erase(i);
        child->Deref();
        return true;
    }

    IComHost* GetParent() const noexcept override {
        return m_parent;
    }

public:
    void* __QueryInterface(const void* type) override {
        auto i = m_impl_table.find(type);
        if (i != m_impl_table.end()) {
            return i->second;
        }
        if (auto* parent = m_parent) {
            return parent->__QueryInterface(type);
        }
        return nullptr;
    }

    void* __Detach(const void* type) override {
        auto i = m_impl_table.find(type);
        if (i != m_impl_table.end()) {
            void* impl = i->second;
            m_impl_table.erase(i);
            return impl;
        }
        return nullptr;
    }

    void* __Attach(const void* type, void* impl) override {
        if (m_impl_table.emplace(type, impl).second) {
            return impl;
        }
        return nullptr;
    }

    bool __SetParent(IComHost* parent) override {
        if (parent && m_parent) {
            return false;
        }
        m_parent = parent;
        return true;
    }

private:
    int m_refc = 1;
    IComHost* m_parent{};
    std::unordered_map<const void*, void*> m_impl_table;
    std::vector<IComHost*> m_children;
};

IComHost* NewComHost(unsigned int hint) {
    return new CComHost(hint);
}

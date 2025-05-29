#include "comet.hpp"

#include <unordered_map>

class CComHost final : public IComHost {
public:
    CComHost(unsigned int hint) {
        if (hint > 0) {
            m_impl_table.reserve(hint);
        }
    }

public:
    void Delete() noexcept override {
        delete this;
    }

    void* __QueryInterface(const void* type) override {
        auto i = m_impl_table.find(type);
        if (i != m_impl_table.end()) {
            return i->second;
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

private:
    std::unordered_map<const void*, void*> m_impl_table;
};

IComHost* NewComHost(unsigned int hint) {
    return new CComHost(hint);
}

#include "comet.hpp"
#include <unordered_map>

class CComHost final : public IComHost {
public:
    void Delete() override {
        delete this;
    }

    void __SetHost(IComHost* host) override {
        // unreachable
    }

    void* __QueryInterface(const std::type_index& type) override {
        auto i = m_impl_table.find(type);
        if (i != m_impl_table.end()) {
            return i->second;
        }
        return nullptr;
    }

    void* __Enject(const std::type_index& type) override {
        auto i = m_impl_table.find(type);
        if (i != m_impl_table.end()) {
            void* impl = i->second;
            m_impl_table.erase(i);
            return impl;
        }
        return nullptr;
    }

    void* __Inject(const std::type_index& type, void* impl) override {
        if (m_impl_table.emplace(type, impl).second) {
            return impl;
        }
        return nullptr;
    }

private:
    std::unordered_map<std::type_index, void*> m_impl_table;
};

IComHost* NewComHost() {
    return new CComHost();
}

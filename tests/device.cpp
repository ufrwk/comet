#include "device.h"
#include <iostream>

class CLoggerModule : public CComUnknown<ILogger> {
public:
    void Log(const std::string& message) override {
        std::cout << "[Logger] " << message << "\n";
    }

    const char* GetDeviceName() const override {
        return "LoggerModule";
    }
};

class CNetworkModule : public CComUnknown<INetwork> {
public:
    void Connect(const std::string& address) override {
        connected = true;
        CComPtr<ILogger> logger(this);
        if (logger) {
            logger->Log("Connecting to: " + address);
        }
    }

    bool IsConnected() const override {
        return connected;
    }

    const char* GetDeviceName() const override {
        return "NetworkModule";
    }

private:
    bool connected = false;
};

void SetupDevice(IComHost* host) {
    host->Attach<ILogger>(new CLoggerModule());
    host->Attach<INetwork>(new CNetworkModule());
};

void RemoveDevice(IComHost* host) {
    auto* p1 = host->Detach<ILogger>();
    delete p1;
    auto* p2 = host->Detach<INetwork>();
    delete p2;
}

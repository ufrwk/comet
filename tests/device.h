#pragma once
#include <comet.hpp>

#include <string>

struct IDevice : IComBase {
    virtual ~IDevice() = default;

    virtual const char* GetDeviceName() const = 0;
};

struct ILogger : IDevice {
    virtual void Log(const std::string& message) = 0;
};

struct INetwork : IDevice {
    virtual void Connect(const std::string& address) = 0;
    virtual bool IsConnected() const = 0;
};

void SetupDevice(IComHost* host);
void RemoveDevice(IComHost* host);

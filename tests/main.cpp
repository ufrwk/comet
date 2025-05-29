#include "device.h"
#include <iostream>

int main(int argc, char* argv[]) {
    IComHost* host = NewComHost();

    SetupDevice(host);
    {
        CComPtr<ILogger> log(host);
        log->Log("Initializing network...");

        CComPtr<INetwork> net(log);
        net->Connect("127.0.0.1");

        CComPtr<ILogger> ptr2ptr(net.get());
        std::cout << "Connected: " << std::boolalpha << net->IsConnected() << "\n";
    }
    RemoveDevice(host);

    host->Delete();
    return 0;
}

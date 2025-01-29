#include "tcp.h"

static const char* TAG = "TCPService";

tcp::tcp(SIM7600& sim7600) : simModule(sim7600) {}

void tcp::activeTcpService() {
    std::string cmd_netopen = simModule.sendCommandWithResponse("AT+NETOPEN", SYSTEM_INIT_DELAY_MS);
}

void tcp::configTcpServer(const std::string& server, int port) {
    std::string cipopen = simModule.sendCommandWithResponse("AT+CIPOPEN=0,\"TCP\",\"" + server + "\"," + std::to_string(port), SYSTEM_INIT_DELAY_MS);
}
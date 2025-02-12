#include "tcp.h"

//static const char* TAG = "TCPService";

tcp::tcp(SIM7600& sim7600) : simModule(sim7600) {}

void tcp::activeTcpService() {
    simModule.sendATCommand("AT+NETOPEN");
}

void tcp::configTcpServer(const std::string& server, int port) {
    simModule.sendATCommand("AT+CIPOPEN=0,\"TCP\",\"" + server + "\"," + std::to_string(port));
}
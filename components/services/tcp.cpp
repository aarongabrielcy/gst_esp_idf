#include "tcp.h"

static const char* TAG = "TCPService";

tcp::tcp(SIM7600& sim7600) : simModule(sim7600) {}

void tcp::activeTcpService() {
    ESP_LOGI(TAG, "Activando servicio TCP...");
    simModule.sendATCommand("AT+NETOPEN");
}

void tcp::configTcpServer(const std::string& server, int port) {
    ESP_LOGI(TAG, "Configurando servidor TCP: %s:%d", server.c_str(), port);
    simModule.sendATCommand("AT+CIPOPEN=0,\"TCP\",\"" + server + "\"," + std::to_string(port));
}
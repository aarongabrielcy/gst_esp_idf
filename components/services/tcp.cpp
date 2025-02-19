#include "tcp.h"

//static const char* TAG = "TCPService";

tcp::tcp(SIM7600& sim7600) : simModule(sim7600) {}

bool tcp::activeTcpService() {
    std:: string response = simModule.sendATCommand("AT+NETOPEN",3000, 0);
    if (response.find("OK") != std::string::npos) {
        ESP_LOGI("TCP COMP", "SERVICIO TCP ACTIVADO.");
        return true;
    }else{
        ESP_LOGI("TCP COMP", "Error al activar TCP =>%s", response.c_str());
        return false;   
    }
}
bool tcp::configTcpServer(const std::string& server, int port) {
    bool stateCMD = false;
    bool stateCFG = false;
    static std::string response;
    response = simModule.sendATCommand("AT+CIPOPEN=0,\"TCP\",\"" + server + "\"," + std::to_string(port), 3000, 0);

    ESP_LOGI("TCP COMP", "respuesta CIPOPEN => %s", response.c_str());
    if(response.find("OK") != std::string::npos) {
        ESP_LOGI("SIM7600", "Comando CIP recibido exitosamente.");
        stateCMD = true;
    }
    if(!validConfigTCP()) {
        ESP_LOGI("SIM7600", "Servidor no configurado.");
        stateCFG = false;
        return false;
    }else {stateCFG = true;}

    return stateCFG && stateCMD ? true : false;
}
bool tcp::validConfigTCP() {
    static std::string response;
    response = simModule.sendATCommand("AT+CIPOPEN?", 3000, 0);
    response  = simModule.cleanATResponse(response, "AT+CIPOPEN?");
    ESP_LOGI("TCP COMP", "VALIDACION CIPOPEN => %s", response.c_str());

    return response.find("-1") != std::string::npos;
}
 /*bool tcp::configTcpServer(const std::string& server, int port) {
    std::string response = simModule.sendATCommand("AT+CIPOPEN=0,\"TCP\",\"" + server + "\"," + std::to_string(port),3000);
    ESP_LOGI("TCP COMP", "respueta CIPOPEN =>%s", response.c_str());
    if (response.find("OK") != std::string::npos) {
        ESP_LOGI("TCP COMP", "CONECTADO CORRECTAMENTE al servidor");
        return true;
    }else {
        ESP_LOGI("TCP COMP", "Error al enviar mensaje TCP =>%s", response.c_str());
        return false;
    }
}*/

bool tcp::sendData(std::string& message) {
    simModule.sendTcpCommand("AT+CIPSEND=0,"+std::to_string(message.length()));
    std::string resUART = simModule.readUART();
    ESP_LOGI("TCP COMP", "Respuesta CIPSEND TCP =>%s", resUART.c_str());
    if (resUART == ">") {
            ESP_LOGI("TCP COMP", "Enviando cadena al servidor");
            simModule.sendATCommand(message,1000, 0);
            return true;
    } else {
        ESP_LOGI("TCP COMP", "Error al enviar mensaje TCP =|%s", resUART.c_str());
        return false;
    }
}

#include "gprsManager.h"

//gprsManager::gprsManager(SIM7600& sim7600) : simModule(sim7600) {}

antennaInfo gprsManager::parseCPSI(const std::string& response) {
  antennaInfo cell;
  if (response.empty()) {
      ESP_LOGE("gprsManager", "Error: parseCPSI recibió un string vacío");
      return cell;
  }

  ESP_LOGW("gprsManager", "parseCPSI => %s", response.c_str());
  
  size_t pos = response.find(",");
  if (pos == std::string::npos) {
      ESP_LOGE("gprsManager", "Error: No se encontró un delimitador en parseCPSI");
      return cell;
  }

  cell.systemMode = response.substr(0, pos);
  ESP_LOGI("gprsManager", "Parsed System Mode:%s", cell.systemMode.c_str());

  if (cell.systemMode == "GSM") {
      parseGSM(response, cell);
  } else if (cell.systemMode == "CDMA") {
      parseCDMA(response, cell);
  } else if (cell.systemMode == "LTE") {
      parseLTE(response, cell);
  } else if (cell.systemMode == "WCDMA") {
      parseWCDMA(response, cell);
  } else if (cell.systemMode == "EVDO") {
      parseEVDO(response, cell);
  } else {
      ESP_LOGW("gprsManager", "NO SERVICE");
  }

  return cell;
}

void gprsManager::parseGSM(const std::string& response, antennaInfo& cell) {
    ESP_LOGI("gprsManager", "Entra a parseGSM");

    // Inicializar la estructura con valores predeterminados
    cell = {"GSM", "", "", "", "", "", 0, 0, 0, 0};

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;

    // Dividir la cadena en tokens usando ','
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 9) {  // Verificar que haya suficientes elementos
        ESP_LOGE("gprsManager", "Error: la respuesta tiene menos de 9 campos.");
        return;
    }

    // Asignar valores en el orden correcto
    cell.operationMode = tokens[1];  // Online / Offline
    cell.mcc = tokens[2].substr(0, 3);  // Extraer MCC
    cell.mnc = tokens[2].substr(4, 2);  // Extraer MNC
    cell.lac = tokens[3];  // LAC (Local Area Code)
    cell.cellId = tokens[4];  // Cell ID
    cell.rxLevel = std::stoi(tokens[6]);  // RxLev
}

void gprsManager::parseWCDMA(const std::string& response, antennaInfo& cell) {
    ESP_LOGI("gprsManager", "Entra a parseWCDMA");

    // Inicializar la estructura con valores predeterminados
    cell = {"WCDMA", "", "", "", "", "", 0, 0, 0, 0};

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    // Dividir la cadena en tokens usando ','
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    if (tokens.size() < 14) {  // Verificar que haya suficientes elementos
        ESP_LOGE("gprsManager", "Error: la respuesta tiene menos de 14 campos.");
        return;
    }
    // Asignar valores en el orden correcto
    cell.operationMode = tokens[1];  // Online / Offline
    cell.mcc = tokens[2].substr(0, 3);  // Extraer MCC
    cell.mnc = tokens[2].substr(4, 2);  // Extraer MNC
    cell.lac = tokens[3];  // LAC (Local Area Code)
    cell.cellId = tokens[4];  // Cell ID
    cell.rxLevel = std::stoi(tokens[12]);  // RxLev
}

void gprsManager::parseLTE(const std::string& response, antennaInfo& cell) {
    ESP_LOGI("gprsManager", "Entra a parseLTE");
    // Inicializar la estructura
    cell = {"LTE", "", "", "", "", "", 0, 0, 0, 0};
    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    // Dividir la cadena en tokens usando ','
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    if (tokens.size() < 14) {  // Asegurar que haya suficientes elementos
        ESP_LOGE("gprsManager", "Error: la respuesta tiene menos de 14 campos.");
        return;
    }
    // Asignar los valores en el orden correcto
    cell.operationMode = tokens[1];  // Online / Offline
    cell.mcc = tokens[2].substr(0, 3);  // Extraer MCC
    cell.mnc = tokens[2].substr(4, 3);  // Extraer MNC
    cell.lac = tokens[3];  // Tracking Area Code (TAC)
    cell.cellId = tokens[4];  // Cell ID
    cell.rsrq = std::stoi(tokens[10]);  // RSRQ
    cell.rxLevel = std::stoi(tokens[11]);  // RSRP
    cell.rssi = std::stoi(tokens[12]);  // RSSI
    cell.rssnr = std::stoi(tokens[13]);  // RSSNR
}

void gprsManager::parseCDMA(const std::string& response, antennaInfo& cell) {
    ESP_LOGI("gprsManager", "Entra a parseCDMA");

    // Inicializar la estructura con valores predeterminados
    cell = {"CDMA", "", "", "", "", "", 0, 0, 0, 0};

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;

    // Dividir la cadena en tokens usando ','
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    if (tokens.size() < 14) {  // Verificar que haya suficientes elementos
        ESP_LOGE("gprsManager", "Error: la respuesta tiene menos de 14 campos.");
        return;
    }
    // Asignar valores en el orden correcto
    cell.operationMode = tokens[1];  // Online / Offline
    if (tokens[2].find('-') != std::string::npos) {  // Si MCC-MNC está presente
        cell.mcc = tokens[2].substr(0, 3);  // Extraer MCC
        cell.mnc = tokens[2].substr(4, 2);  // Extraer MNC
    }
    cell.rxLevel = std::stoi(tokens[6]);  // CDMA RX Chain 0 AGC
}

void gprsManager::parseEVDO(const std::string& response, antennaInfo& cell) {
    ESP_LOGI("gprsManager", "Entra a parseEVDO");

    // Inicializar la estructura con valores predeterminados
    cell = {"EVDO", "", "", "", "", "", 0, 0, 0, 0};

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;

    // Dividir la cadena en tokens usando ','
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 10) {  // Verificar que haya suficientes elementos
        ESP_LOGE("gprsManager", "Error: la respuesta tiene menos de 10 campos.");
        return;
    }

    // Asignar valores en el orden correcto
    cell.operationMode = tokens[1];  // Online / Offline
    if (tokens[2].find('-') != std::string::npos) {  // Si MCC-MNC está presente
        cell.mcc = tokens[2].substr(0, 3);  // Extraer MCC
        cell.mnc = tokens[2].substr(4, 2);  // Extraer MNC
    }
    cell.rxLevel = std::stoi(tokens[5]);  // EVDO RX Chain 0 AGC
}


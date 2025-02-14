#include "gpsManager.h"

gnssInfo gpsManager::parseGNSS(const std::string& response) {
    gnssInfo gnss;
    ESP_LOGI("gprsManager", "Entra a parseGNSS");
    
        // Inicializar estructura con valores predeterminados
        gnss = {};
    
        std::vector<std::string> tokens;
        std::stringstream ss(response);
        std::string token;
    
        // Dividir la cadena en tokens usando ','
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
    
        // Si la cadena solo tiene comas vacías o menos de 16 elementos, no hay fix
        if (tokens.size() < 16 || response.find_first_not_of(",") == std::string::npos) {
            gnss.fix = 0;
            ESP_LOGW("gprsManager", "No hay fix de GNSS, valores por defecto asignados.");
            return gnss;
        }
    
        // Asignar directamente los valores en string
        gnss.mode = std::stoi(tokens[0]);
        gnss.gps_svs = std::stoi(tokens[1]);
        gnss.glonass_svs = std::stoi(tokens[2]);
        gnss.beidou_svs = std::stoi(tokens[3]);
        gnss.lat = std::stod(tokens[4]);
        gnss.ns = tokens[5].empty() ? 'N' : tokens[5][0]; // Validar que no esté vacío
        gnss.log = std::stod(tokens[6]);
        gnss.ew = tokens[7].empty() ? 'E' : tokens[7][0]; // Validar que no esté vacío
        gnss.date = tokens[8];
        gnss.utc_time = tokens[9];
        gnss.alt = std::stod(tokens[10]);
        gnss.speed = std::stod(tokens[11]);
        gnss.course = std::stod(tokens[12]);
        gnss.pdop = std::stod(tokens[13]);
        gnss.hdop = std::stod(tokens[14]);
        gnss.vdop = std::stod(tokens[15]);
        gnss.fix = 1;
    
    return gnss;
}
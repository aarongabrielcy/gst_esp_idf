#include "utils/simUtils.h"

std::string extractIMEI(const std::string& response) {
    // Remover ",OK"
    size_t pos = response.find(",OK");
    std::string cleaned = (pos != std::string::npos) ? response.substr(0, pos) : response;
    // Extraer los últimos 10 dígitos
    if (cleaned.length() > 10) {
        cleaned = cleaned.substr(cleaned.length() - 10);
    }
    return cleaned;
}
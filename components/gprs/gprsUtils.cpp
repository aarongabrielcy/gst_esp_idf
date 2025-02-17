#include "gprsUtils/gprsUtils.h"

std::string formatLac(std::string lac) {
    // Verifica si la cadena comienza con "0x"
    if (lac.rfind("0x", 0) == 0) {  // rfind devuelve 0 si "0x" está al inicio
        return lac.substr(2);  // Elimina los dos primeros caracteres
    }
    return lac;
}
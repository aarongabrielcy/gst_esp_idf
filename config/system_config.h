#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// Configuración de tiempos
#define SYSTEM_TASK_DELAY_MS 1000
#define SYSTEM_INIT_DELAY_MS 3000

// Configuración de red
#define DEFAULT_APN "internet.itelcel.com"
#define SERVER_URL "34.196.135.179"
#define SERVER_PORT 5200

struct Headers {
    static constexpr const char *STT = "STT";
    static constexpr const char *ALT = "ALT";
    static constexpr const char *ALV = "ALV";
    static constexpr const char *RES = "RES";
    static constexpr const char *CMD = "CMD";
};
#define DLM ";" //delimiter
#define CHANGE_COURSE 20
#endif // SYSTEM_CONFIG_H
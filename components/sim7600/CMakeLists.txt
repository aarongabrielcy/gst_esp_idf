idf_component_register(
    SRCS "SIM7600.cpp" "utils/simUtils.cpp"
    INCLUDE_DIRS "include" "../../config" "utils"
    REQUIRES driver esp_timer gprs gps
)
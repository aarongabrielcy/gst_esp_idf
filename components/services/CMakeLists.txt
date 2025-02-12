idf_component_register(
    SRCS "sms.cpp" "tcp.cpp"
    INCLUDE_DIRS "include" "../../config"
    REQUIRES sim7600
)

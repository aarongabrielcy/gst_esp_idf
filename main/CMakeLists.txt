idf_component_register(
    SRCS "main.cpp"                
    PRIV_REQUIRES spi_flash
    INCLUDE_DIRS "." "../config"
    REQUIRES sim7600 pwModule basic services gps gprs
)

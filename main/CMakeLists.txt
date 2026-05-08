idf_component_register(SRCS "main.c" "gateway_lora.c"
                       INCLUDE_DIRS "."
                       REQUIRES sx127x_driver)
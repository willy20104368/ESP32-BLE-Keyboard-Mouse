idf_component_register(SRCS "main.c"
                            "button.c"
                            "joystick.c"
                            "esp_hidd_prf_api.c"
                            "hid_dev.c"
                            "hid_device_le_prf.c"
                    PRIV_REQUIRES bt nvs_flash esp_driver_gpio esp_adc
                    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)

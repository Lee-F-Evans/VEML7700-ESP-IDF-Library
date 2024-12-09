set(srcs)

idf_component_register(SRCS "veml7700.c" "main.c" "${srcs}"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver soft_i2c_master)

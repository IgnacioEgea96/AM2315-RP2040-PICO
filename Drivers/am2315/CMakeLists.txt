add_library(am2315 INTERFACE)

target_include_directories(am2315
    INTERFACE
    ./include)

target_sources(am2315
    INTERFACE
    am2315.c
)

target_link_libraries(am2315
    INTERFACE
    hardware_i2c
)
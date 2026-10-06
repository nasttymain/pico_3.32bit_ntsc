add_library(pico_3bit32_ntsc INTERFACE)

set(dir ${CMAKE_CURRENT_LIST_DIR})

pico_generate_pio_header(pico_3bit32_ntsc ${CMAKE_CURRENT_LIST_DIR}/cvbs_output.pio)
pico_generate_pio_header(pico_3bit32_ntsc ${CMAKE_CURRENT_LIST_DIR}/cvbs_extras/picopico_sound/picopico.pio)

target_link_libraries(pico_3bit32_ntsc INTERFACE
    pico_stdlib
    hardware_pio
    hardware_dma
    pico_multicore
)

target_include_directories(pico_3bit32_ntsc INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(pico_3bit32_ntsc INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/cvbs/cvbs_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cvbs/cvbs_core.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cvbs/cvbs_write.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cvbs/cvbs_bezier.cpp
)

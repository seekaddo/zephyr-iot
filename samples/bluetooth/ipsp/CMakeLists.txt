# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ipsp)

if (0)
    target_sources(app PRIVATE
            src/main.c
            )
else()

    file(GLOB quant_lib
            ${PROJECT_SOURCE_DIR}/quant/*.c
            ${PROJECT_SOURCE_DIR}/quant/warpe/*.c
            ${PROJECT_SOURCE_DIR}/quant/deps/*.c
            ${PROJECT_SOURCE_DIR}/quant/deps/extra/*.c
            )

    set (QUANT_INC
            ${PROJECT_SOURCE_DIR}/quant
            ${PROJECT_SOURCE_DIR}/quant/warpe
            ${PROJECT_SOURCE_DIR}/quant/deps
            )

    target_include_directories(app PRIVATE "${QUANT_INC}")
    #set (BUIDFLAG )
    #set (BUIDFLAG1 ) # -D'alloca(x)=(void*)&x')
    #    -std=c11 -Os -fstack-usage -foptimize-strlen -ffast-math)
    #add_compile_options(-std=c11 -Os -fstack-usage -foptimize-strlen -ffast-math "-D'alloca(x)=(void*)&x'")
    add_compile_options(-std=c11 -Os -fstack-usage -foptimize-strlen -ffast-math "-D'alloca(x)=(void*)&x'")
    target_sources(app PRIVATE
            src/main.c
            src/mini_transaction.c
            ${quant_lib}
            )

    set (DEFINES
            MINIMAL_CIPHERS
            NO_ERR_REASONS
            NO_OOO_0RTT
            NO_MIGRATION
            NO_SRT_MATCHING
            NO_QLOG
            NO_TLS_LOG
            NO_ECN
            RIOT_VERSION
            CONTIKI_NG_LE
            DLEVEL=DBG
            ZEPHYR_TEST
            )
    set_property(DIRECTORY . APPEND PROPERTY COMPILE_DEFINITIONS ${DEFINES})

    #add_compile_definitions( ${BUIDFLAG})
    #add_compile_definitions( ${BUIDFLAG1})
endif ()





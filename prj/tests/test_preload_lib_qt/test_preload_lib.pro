
TEMPLATE = lib

#DEFINES += DO_LIB_DEBUG

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += $${PWD}/../../../include

SOURCES += \
    $${PWD}/../../../src/libs/dllmain_preload_for_log_correction.cpp           \
    $${PWD}/../../../src/libs/preload_for_log_correction_env_based.cpp


HEADERS += \
    $${PWD}/../../../include/preload_for_log_correction.h                       \
    $${PWD}/../../../include/preload_lib_internal.h

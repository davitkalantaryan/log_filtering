
TEMPLATE = lib

win32{

} else {

    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    QMAKE_CXXFLAGS += -std=c++11
}

message("!!!!!!!!!!! log_filtering_base.pro")
include( $${PWD}/../../common/common_qt/sys_common.pri )

#DEFINES += DO_LIB_DEBUG

INCLUDEPATH += $${PWD}/../../../include

SOURCES += \
    $${PWD}/../../../src/libs/dllmain_preload_for_log_correction.cpp



HEADERS += \
    $${PWD}/../../../include/preload_for_log_correction.h                       \
    $${PWD}/../../../include/preload_lib_internal.h


OTHER_FILES += \
    $${PWD}/../../../src/libs/preload_for_log_correction_env_based.cpp

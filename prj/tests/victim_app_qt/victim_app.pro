
win32{

} else {

    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    QMAKE_CXXFLAGS += -std=c++11
}

message("!!!!!!!!!!! victim_app.pro")
include( $${PWD}/../../common/common_qt/sys_common.pri )

SOURCES += \
    $${PWD}/../../../src/tests/main_victim_app.cpp


win32{

} else {

    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    QMAKE_CXXFLAGS += -std=c++11
}

QT -= gui
QT -= core
QT -= widgets
CONFIG -= qt

message("!!!!!!!!!!! victim_app.pro")
include( $${PWD}/../../common/common_qt/sys_common.pri )

SOURCES += \
    $${PWD}/../../../src/tests/main_test_doocs_server.cpp

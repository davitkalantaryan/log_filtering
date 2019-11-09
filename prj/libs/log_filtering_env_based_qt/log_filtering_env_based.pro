
TEMPLATE = lib

win32{

} else {

    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    QMAKE_CXXFLAGS += -std=c++11
}

message("!!!!!!!!!!! log_filtering_env_based.pro")
include( $${PWD}/../../common/common_qt/sys_common.pri )

INCLUDEPATH += $${PWD}/../../../include
INCLUDEPATH += $${PWD}/../../../contrib/cpp-raft/include

SOURCES += \
    $${PWD}/../../../src/libs/dllmain_preload_for_log_correction.cpp           \
    $${PWD}/../../../src/libs/preload_for_log_correction_env_based.cpp


HEADERS += \
    $${PWD}/../../../include/preload_for_log_correction.h                       \
    $${PWD}/../../../include/preload_lib_internal.h                             \
    $${PWD}/../../../contrib/cpp-raft/include/common/hashtbl.hpp                \
    $${PWD}/../../../contrib/cpp-raft/include/common/hashtbl.impl.hpp


OTHER_FILES += \
    $${PWD}/../../../contrib/cpp-raft/include/common/common_hashtbl.hpp         \
    $${PWD}/../../../contrib/cpp-raft/include/common/impl.common_hashtbl.hpp

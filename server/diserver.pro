QT += core gui widgets websockets svg

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17 strict_c++ optimize_full lrelease embed_translations winqtdeploy

SOURCES += \
    src/config.cpp \
    src/main.cpp \
    src/diserver.cpp \
    src/server.cpp \
    src/ui.cpp \
    src/utils.cpp

HEADERS += \
    include/config.hpp \
    include/diserver.hpp \
    include/utils.hpp \

INCLUDEPATH += include/

FORMS += \
    diserver.ui

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/zh_TW.ts \
    translations/en.ts

msvc:QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

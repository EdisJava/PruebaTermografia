QT += gui
QT += sql

TEMPLATE = lib
DEFINES += CORE_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AnalysisEngine.cpp \
    IngestionEngine.cpp \
    TermografiaManager.cpp \
    daos.cpp

HEADERS += \
    Core_global.h \
    daos.h \
    engines.h \
    entities.h \
    termografiamanager.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

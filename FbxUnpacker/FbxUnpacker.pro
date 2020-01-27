TEMPLATE = app

CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    geom/boundingBox.cpp \
    main.cpp \
    fbxConvert.cpp \
    weights.cpp

## LIB FBX configurations

#Mac
macx: LIBS += -L'/Applications/Autodesk/FBX SDK/2020.0.1/lib/clang/release/' -lfbxsdk
macx: INCLUDEPATH +='/Applications/Autodesk/FBX SDK/2020.0.1/include/'

## End FBX configurations

HEADERS += \
    fbxConvert.h \
    geom/boundingBox.h \
    geom/vec3.h \
    weights.h

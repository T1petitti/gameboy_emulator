HEADERS += Z80.h
HEADERS += screen.h
SOURCES += gameboy.cpp
SOURCES += Z80.cpp
SOURCES += screen.cpp

QT += core widgets

target.path = gameboy
sources.files= $$SOURCES $$HEADERS gameboy.pro
sources.path = .
INSTALLS += target sources


TEMPLATE = lib

QT       += core serialport

# either SP_LIBRARY or SP_STATIC_LIBRARY
DEFINES += SP_LIBRARY

HEADERS += \
	src/SP_SerialPortCommunicator.h \
	src/SP_SerialPortWorker.h \
	src/SP_global.h

SOURCES += \
	src/SP_SerialPortCommunicator.cpp \
	src/SP_SerialPortWorker.cpp

#-------------------------------------------------
#
# Project created by QtCreator 2017-11-04T12:53:22
#
#-------------------------------------------------

QT       += core network sql
QT +=       xml

QT       -= gui

TARGET = RpiWdg
CONFIG   += console exceptions
CONFIG   -= app_bundle
CONFIG   += c++11
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
DESTDIR = ../Build_RPI/RpiWdg_Folder #Target file directory

OBJECTS_DIR = ../Build_RPI_Objects #Intermediate object files directory

MOC_DIR = ../Build_RPI_Objects #Intermediate moc files directory

SOURCES += main.cpp \
    controller.cpp \
    Tcp_Client.cpp \
    spi_cnn.cpp \
    bcm2835.c \
    waitresponse.cpp \
    smscontroller.cpp \
    broadcastrcv.cpp \
    tc_centserver.cpp \
    cSPI.cpp \
    Lan_Conn.cpp \
    MemoryMap.cpp \
    Messaging.cpp \
    Sin_array.cpp \
    Slave_Card.cpp \
    SPI_Conn.cpp \
    db_handler.cpp \
    telegrambot.cpp

HEADERS += \
    controller.h \
    Tcp_Client.h \
    spi_cnn.h \
    CommonDefines.h \
    CommIf_SiemensST.h \
    waitresponse.h \
    smscontroller.h \
    broadcastrcv.h \
    tc_centserver.h \
    bcm2835.h \
    cSPI.h \
    Lan_Conn.h \
    MemoryMap.h \
    Messaging.h \
    Sin_array.h \
    Slave_Card.h \
    SPI_Conn.h \
    db_handler.h \
    telegrambot.h

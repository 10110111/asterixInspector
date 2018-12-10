#-------------------------------------------------
#
# Project created by QtCreator 2010-10-24T17:35:52
#
#-------------------------------------------------

QT       += widgets gui xml webenginewidgets network

QMAKE_CXXFLAGS += -std=c++11

TARGET = asterixInspector
TEMPLATE = app


SOURCES += main.cpp\
        mainWindow.cpp \
    asterix.cpp \
    asterixFileMapper.cpp \
    asterixModel.cpp \
    asterixReportGenerator.cpp \
    hexedit.cpp \
    global.cpp \
    uap.cpp \
    dataItemWidget.cpp \
    recordWidget.cpp \
    scale_expression.bison.cpp \
    scale_expression.flex.cpp \
    scaleExpressionParser.cpp \
    specificationSelectDialog.cpp \
    TextHexDumpEdit.cpp \
    logger.cpp

HEADERS  += mainWindow.h \
    asterix.h \
    asterixFileMapper.h \
    asterixModel.h \
    asterixReportGenerator.h \
    hexedit.h \
    global.h \
    uap.h \
    dataItemWidget.h \
    recordWidget.h \
    scale_expression.bison.h \
    scale_expression.flex.h \
    scaleExpressionParser.h \
    specificationSelectDialog.h \
    logger.h \
    TextHexDumpEdit.h \
    bit.h

FORMS    += mainWindow.ui \
    TextHexDumpEdit.ui \
    specificationSelectDialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    dataItem.css \
    asterixSpecification/cat8.xml \
    asterixSpecification/cat1_v1.1.xml \
    asterixSpecification/cat2_v1.0.xml \
    asterixSpecification/cat4_v1.4.xml \
    asterixSpecification/cat7_v1.5.xml \
    asterixSpecification/cat10_v1.1.xml \
    asterixSpecification/cat11_v1.2.xml \
    asterixSpecification/cat20_v1.8.xml \
    asterixSpecification/cat21_v2.1.xml \
    asterixSpecification/cat23_v1.2.xml \
    asterixSpecification/cat34_v1.27.xml \
    asterixSpecification/cat48_v1.20.xml \
    asterixSpecification/cat62_v1.12.xml \
    asterixSpecification/cat63_v1.3.xml \
    asterixSpecification/cat64_v1.3.xml

# gui lib

cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0071) # Let AUTOMOC and AUTOUIC process GENERATED files.
  cmake_policy(SET CMP0071 NEW)
endif()

set(GUI_LIB_HEADERS
  AboutDialog.h
  Network.h
  NetworkSignal.h
)
set(GUI_LIB_SOURCES

  AutoUpdater.cc
  CGALWorker.cc
  ViewportControl.cc
  Console.cc
  Dock.cc
  Editor.cc
  ErrorLog.cc
  EventFilter.h
  FontListDialog.cc
  FontListTableView.cc
  InitConfigurator.cc
  LaunchingScreen.cc
  LibraryInfoDialog.cc
  MainWindow.cc
  Animate.cc
  MouseSelector.cc
  OctoPrint.cc
  OpenCSGWarningDialog.cc
  OpenSCADApp.cc
  Preferences.cc
  PrintInitDialog.cc
  PrintService.cc
  ProgressWidget.cc
  QGLView.cc
  QGLView2.cc
  QSettingsCached.cc
  QWordSearchField.cc
  ScadApi.cc
  ScadLexer.cc
  ScintillaEditor.cc
  Settings.cc
  SettingsWriter.cc
  TabManager.cc
  TabWidget.cc
  UIUtils.cc
  WindowManager.cc
  IgnoreWheelWhenNotFocused.cc
  input/AxisConfigWidget.cc
  input/ButtonConfigWidget.cc
  input/InputDriver.cc
  input/InputDriverManager.cc
  input/InputEventMapper.cc
  parameter/GroupWidget.cc
  parameter/ParameterCheckBox.cc
  parameter/ParameterComboBox.cc
  parameter/ParameterSlider.cc
  parameter/ParameterSpinBox.cc
  parameter/ParameterText.cc
  parameter/ParameterVector.cc
  parameter/ParameterVirtualWidget.cc
  parameter/ParameterWidget.cc
)

add_library(openscadGUI ${GUI_LIB_HEADERS} ${GUI_LIB_SOURCES})

find_package(Qt5 5.4 COMPONENTS Core Widgets Multimedia OpenGL Concurrent Network Svg REQUIRED QUIET)

foreach(library openscadGUI )
  if("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.14")
    set_property(TARGET ${library} PROPERTY AUTOGEN_ORIGIN_DEPENDS OFF)
  endif()
  set_property(TARGET ${library} PROPERTY AUTOMOC ON)
  set_property(TARGET ${library} PROPERTY AUTOUIC ON)
  set_property(TARGET ${library} PROPERTY AUTORCC ON)
  set_property(TARGET ${library} PROPERTY AUTOUIC_OPTIONS --tr q_)
  set_property(TARGET ${library} PROPERTY AUTORCC_OPTIONS --threshold 10 --compress 9)

  if (Qt5_POSITION_INDEPENDENT_CODE)
    set_property(TARGET ${library} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  if (("${Qt5_VERSION}" VERSION_GREATER_EQUAL "5.4"))
    target_compile_definitions(${library} PRIVATE USE_QOPENGLWIDGET)
  endif()
endforeach(library)

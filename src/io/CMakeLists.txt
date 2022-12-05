#io lib

set(IO_LIB_SOURCES
  DxfData.cc
  dxfdim.cc
  export.cc
  export_3mf.cc
  export_amf.cc
  export_dxf.cc
  export_obj.cc
  export_off.cc
  export_wrl.cc
  export_pdf.cc
  export_stl.cc
  export_svg.cc
  export_nef.cc
  export_param.cc
  fileutils.cc
  import_3mf.cc
  import_amf.cc
  import_stl.cc
  import_off.cc
  import_svg.cc
  import_json.cc
  import_nef.cc
  libsvg/circle.cc
  libsvg/data.cc
  libsvg/ellipse.cc
  libsvg/group.cc
  libsvg/libsvg.cc
  libsvg/line.cc
  libsvg/path.cc
  libsvg/polygon.cc
  libsvg/polyline.cc
  libsvg/rect.cc
  libsvg/shape.cc
  libsvg/svgpage.cc
  libsvg/text.cc
  libsvg/transformation.cc
  libsvg/tspan.cc
  libsvg/use.cc
  libsvg/util.cc
)

add_library(openscadIO ${IO_LIB_SOURCES})


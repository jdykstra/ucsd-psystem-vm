xturtleserver_sources = [
  'main.c',
]

xturtleserver_dependencies = [
  m_dep,
  libexplain_dep,
  x11_dep,
  xt_dep,
]

xturtleserver_exe = executable(
  'ucsdpsys_xturtleserver',
  sources : xturtleserver_sources,
  include_directories : root_inc,
  implicit_include_directories : false,
  dependencies : xturtleserver_dependencies,
  link_with : lib_lib,
  install : true,
)

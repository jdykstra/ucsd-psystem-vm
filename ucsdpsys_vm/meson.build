vm_sources = [
  'array.c',
  'long_integer.c',
  'main.c',
  'native6502.c',
  'printer.c',
  'ptrace.c',
  'search.c',
  'sets.c',
  'stack.c',
  'term.c',
  'turtlegr.c',
  'unit_io.c',  
]

vm_dependencies = [
  m_dep,
  libexplain_dep,
  curses_dep,
]

vm_exe = executable(
  'ucsdpsys_vm',
  sources : vm_sources,
  include_directories : root_inc,
  implicit_include_directories : false,
  dependencies : vm_dependencies,
  link_with : lib_lib,
  install : true,
)

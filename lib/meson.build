lib_sources = [
    'byte_sex.c',
    'diskio.c',
    'memory.c',
    'progname.c',
    'version.c',
    'version_stamp.c',
]

lib_lib = static_library(
  'libucsd-psystem-vm',
  sources : lib_sources,
  include_directories : root_inc,
  implicit_include_directories : false,
  dependencies : libexplain_dep,
  install : false,
)

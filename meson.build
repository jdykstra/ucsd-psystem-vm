project('ucsd-psystem-vm', 'c')

fs = import('fs')

cc = meson.get_compiler('c')
sh = find_program('sh', '/bin/sh')

m_dep = cc.find_library('m', required : false)
ncurses_dep = dependency('ncurses')
libexplain_dep = dependency('libexplain')
x11_dep = dependency('x11')
xt_dep = dependency('xt')

root_inc = include_directories('.')

subdir('lib')

subdir('ucsdpsys_vm')
subdir('ucsdpsys_xturtleserver')

subdir('man/man1')

subdir('test/00')

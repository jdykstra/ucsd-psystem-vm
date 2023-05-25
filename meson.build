project('ucsd-psystem-vm', 'c', meson_version: '>=0.58.0')

fs = import('fs')

cc = meson.get_compiler('c')
sh = find_program('sh')

m_dep = cc.find_library('m', required : false)
curses_dep = dependency('curses')
libexplain_dep = dependency('libexplain')
x11_dep = dependency('x11')
xt_dep = dependency('xt')

root_inc = include_directories('.')

subdir('lib')

subdir('ucsdpsys_vm')
subdir('ucsdpsys_xturtleserver')

subdir('man/man1')

subdir('test/00')

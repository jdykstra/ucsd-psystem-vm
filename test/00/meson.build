env = environment()
env.prepend('PATH', fs.parent(vm_exe.full_path()))
env.prepend('PATH', meson.project_source_root() / 'script')

depends = [vm_exe]

cases = [
  't0001a',
  't0003a',
  't0002a',
  't0005a',
  't0006a',
  't0007a',
  't0008a',
  't0009a',
  't0010a',
  't0011a',
  't0012a',
  't0015a',
  't0016a',
  't0017a',
  't0018a',
  't0019a',
  't0020a',
]

foreach name : cases
  test(name, sh, args : files(name + '.sh'), env : env, depends : depends)
endforeach

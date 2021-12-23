project('hello-gum', 'c',
  default_options: [
    'c_std=gnu99',
    'default_library=static',
    'optimization=s',
    'b_ndebug=true',
    'strip=true',
  ],
)

host_os = host_machine.system()
if host_os == 'android'
  host_os_family = 'linux'
else
  host_os_family = host_os
endif

cc = meson.get_compiler('c')

have_msvc = cc.get_id() == 'msvc'

extra_c_args = []
extra_link_args = []

# Get rid of unused code.
if host_os_family == 'darwin'
  extra_link_args += '-Wl,-dead_strip'
elif not have_msvc
  extra_c_args += ['-ffunction-sections', '-fdata-sections']
  extra_c_args += cc.get_supported_arguments('-fno-asynchronous-unwind-tables')
  extra_link_args += '-Wl,--gc-sections'
endif

# Ensure no symbols are visible, so the static linker can eliminate more code.
if host_os_family == 'darwin'
  symlist = 'no-exports.symbols'
  extra_link_args += '-Wl,-exported_symbols_list,' + meson.current_source_dir() / symlist
elif not have_msvc
  symscript = 'no-exports.version'
  extra_link_args += '-Wl,--version-script,' + meson.current_source_dir() / symscript
endif

add_global_arguments(extra_c_args, language: 'c')
add_project_link_arguments(extra_link_args, language: 'c')

gum_dep = dependency('frida-gum-1.0', default_options: ['diet=true'])

executable('hello-gum', 'hello.c',
  dependencies: [gum_dep],
  install: true,
)

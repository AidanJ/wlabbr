project('wlabbr', 'c',
        default_options: [
          'default_library=static',
          'c_std=c11',
          ])

add_project_arguments(meson.get_compiler('c').get_supported_arguments([
  '-Wno-c11-c2x-compat'
]) , language: 'c')

src_files = [
  'src/main.c',
  'src/input.h',
  'src/input.c',
  'src/log.h',
  'src/log.c',
  'src/config.h',
  'src/config.c',
]

wayland_client = dependency('wayland-client')
subdir('protocols')

executable(
  'wlabbr',
  files(src_files),
  dependencies: [
    dependency('libcjson'),
    client_protos,
  ]
)

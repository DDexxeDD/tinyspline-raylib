project('tinyspline', 'c')

cc = meson.get_compiler('c')

raylib_directory = meson.current_source_dir() + '/external/raylib'

tinyspline_dependencies = [
  cc.find_library('m'),
  cc.find_library('dl'),
  cc.find_library('pthread'),
  cc.find_library('raylib', dirs : raylib_directory),
]

include = [
  include_directories('.'),
  include_directories('external/tinyspline'),
  include_directories('external/raylib'),
  include_directories('external/raylib_nuklear'),
  include_directories('external/nuklear'),
  include_directories('external/cvector'),
]

source_files = [
  'source/implementations.c',
  'source/main.c',
  'source/common.c',
  'source/demo_samples.c',
  'source/demo_eval.c',
  'source/demo_interpolation.c',
  'source/demo_frames.c',
]

subdir ('external/tinyspline')

tinyspline_binary = executable (
  'tinyspline',
  source_files,
  include_directories : include,
  dependencies : [tinyspline_dependencies],
  # use float precision in tinyspline so it plays nice with raylib and nuklear
  c_args : '-DTINYSPLINE_FLOAT_PRECISION'
)

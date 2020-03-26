test_unit = 'test-start-menu'

message('source_root:' + meson.source_root())

sources = files(
  test_unit + '.c',
  meson.build_root() + '/src/start-menu/kiran-start-menu-generated.c'
)

exe = executable(
           test_unit,
             sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
         c_args : cflags
)

test(test_unit, exe)

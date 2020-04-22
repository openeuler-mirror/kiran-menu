test_unit = 'test-start-menu'

sources = files(
  test_unit + '.c',
  'test-app-category.c',
  'test-app-favorite.c',
  'test-app-search.c',
  'test-all-apps.c'
)

test_exe = executable(
           test_unit,
             sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
         c_args : cflags,
         link_with : sub_libs
)

test(test_unit, test_exe)

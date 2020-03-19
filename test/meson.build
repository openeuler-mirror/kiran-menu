
test_unit = 'test-start-menu'

sources = files(
  test_unit + '.c'
)


exe = executable(
           test_unit,
             sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
)

test(test_unit, exe)
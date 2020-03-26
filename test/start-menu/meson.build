test_unit_client = 'test-start-menu-client'

client_sources = files(
  test_unit_client + '.c',
  'test-app-category.c',
  'test-app-favorite.c',
  'test-app-frequent.c'
)

client_exe = executable(
           test_unit_client,
             client_sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
         c_args : cflags,
         link_with : sub_libs
)

test(test_unit_client, client_exe)


test_unit_server = 'test-start-menu-server'
server_sources = files(
  test_unit_server + '.c',
  'test-all-apps.c',
)

server_exe = executable(
           test_unit_server,
             server_sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
         c_args : cflags,
         link_with : sub_libs
)

test(test_unit_server, server_exe)

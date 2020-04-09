
sources = files(
  'helper.c'
)

sub_libs += static_library(
          'menu-utils',
              sources : sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
               c_args : cflags
)
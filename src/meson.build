gnome = import('gnome')



subdir('start-menu')

sources = files(
  'main.c'
)

executable(
  meson.project_name(),
               sources,
  include_directories : top_inc,
         dependencies : common_deps,
            link_with : sub_libs,
              install : true,
             link_args: '-g',
          install_dir : libexecdir
)

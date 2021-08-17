project('ogl-demo', 'c', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

gfx_dep = [ dependency('glm'), dependency('OpenGL'), dependency('glfw3') ]
dl_dep = meson.get_compiler('c').find_library('dl')
incdir = include_directories('include')

executable('model-renderer',
           'main.cpp',
           'objs.cpp',
           'fbo.cpp',
           'lib/glad.c',
           dependencies : [
             gfx_dep,
             dl_dep,
             ],
           include_directories: incdir,
           c_args : '-xc++',
           install : true)

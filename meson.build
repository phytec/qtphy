project(
  'qtphy', 'cpp',
  version : '0.4.0',
  license : 'MIT',
  meson_version : '>=0.63.0'
)

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules : ['Core', 'Qml', 'Quick', 'Gui', 'DBus', 'Multimedia'])
exec_dep = [qt6_dep, dependency('gstreamer-1.0'), dependency('gstreamer-pbutils-1.0')]

headers = [
  'src/device_info.hpp',
  'src/rauc.hpp',
  'src/multimedia_formats.hpp'
]
src = [
  'src/main.cpp',
  'src/device_info.cpp',
  'src/rauc.cpp',
  'src/multimedia_formats.cpp'
]

qmlsink_option = get_option('qmlsink')
if qmlsink_option.enabled()
    add_project_arguments('-DQML_SINK', language : 'cpp')
    headers += ['src/multimedia_qmlsink.hpp']
    src += ['src/multimedia_qmlsink.cpp']
endif

inc = include_directories('src')
moc = qt6.compile_moc(
  headers : headers,
  include_directories : inc,
  dependencies : qt6_dep
)
res = qt6.compile_resources(
  sources : 'resources/resources.qrc'
)

executable(
  meson.project_name(),
  sources : [src, res, moc],
  include_directories : inc,
  dependencies : exec_dep,
  install : true
)

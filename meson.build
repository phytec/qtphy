project(
  'qtphy', 'cpp',
  version : '0.3.1',
  license : 'MIT',
  meson_version : '>=0.63.0'
)

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules : ['Core', 'Qml', 'Quick', 'Gui', 'DBus', 'Multimedia'])
gst_dep = dependency('gstreamer-1.0', fallback : ['gstreamer', 'gst_dep'])

headers = [
  'src/device_info.hpp',
  'src/rauc.hpp',
  'src/multimedia_qmlsink.hpp'
]
src = [
  'src/main.cpp',
  'src/device_info.cpp',
  'src/rauc.cpp',
  'src/multimedia_qmlsink.cpp'
]

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
  dependencies : [qt6_dep, gst_dep],
  install : true
)

project(
  'qtphy', 'cpp',
  version : '0.2.1',
  license : 'MIT',
  meson_version : '>=0.63.0'
)

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules : ['Core', 'Qml', 'Quick', 'Gui', 'DBus', 'Multimedia'])

headers = [
  'src/device_info.hpp',
  'src/rauc.hpp'
]
src = [
  'src/main.cpp',
  'src/device_info.cpp',
  'src/rauc.cpp'
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
  dependencies : qt6_dep,
  install : true
)

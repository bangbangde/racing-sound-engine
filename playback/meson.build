playback_inc = include_directories('include', 'src', '../third_party')

cpp = meson.get_compiler('cpp')

# Platform-specific dependencies
platform_deps = []
if host_machine.system() == 'windows'
  platform_deps += cpp.find_library('winmm', required : true)
  platform_deps += cpp.find_library('ole32', required : true)
  platform_deps += cpp.find_library('advapi32', required : true)
elif host_machine.system() == 'linux'
  platform_deps += dependency('threads')
  platform_deps += cpp.find_library('m', required : false)
  platform_deps += cpp.find_library('dl', required : false)
elif host_machine.system() == 'android'
  platform_deps += cpp.find_library('OpenSLES', required : false)
elif host_machine.system() == 'darwin'
  platform_deps += dependency('appleframeworks', modules : ['AudioToolbox', 'CoreAudio', 'CoreFoundation'])
endif

playback_sources = files(
  'src/audio_playback.cpp',
  'src/wav_loader.cpp',
  'src/miniaudio_impl.cpp',
)

playback_lib = static_library('engin-playback',
  playback_sources,
  include_directories : [playback_inc, engine_core_inc],
  dependencies : [engine_core_dep, platform_deps],
)

playback_dep = declare_dependency(
  link_with : playback_lib,
  include_directories : [playback_inc, engine_core_inc],
  dependencies : [engine_core_dep, platform_deps],
)

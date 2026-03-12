project('racing-sound-engine', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++17',
    'warning_level=2',
    'optimization=2',
    'b_lto=true',
  ])

cpp = meson.get_compiler('cpp')

# Platform-specific dependencies
platform_deps = []
platform_args = []

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

# Build options
sample_rate = get_option('sample_rate')
buffer_frames = get_option('buffer_frames')

config_data = configuration_data()
config_data.set('ENGIN_SAMPLE_RATE', sample_rate)
config_data.set('ENGIN_BUFFER_FRAMES', buffer_frames)

# Include directories
inc_dirs = include_directories('src', 'third_party')

# Core library sources
core_sources = files(
  'src/audio/miniaudio_impl.cpp',
  'src/audio/audio_engine.cpp',
  'src/core/engine_model.cpp',
  'src/core/cylinder_bank.cpp',
  'src/synth/harmonic_synth.cpp',
  'src/synth/noise_generator.cpp',
  'src/synth/sample_player.cpp',
  'src/synth/turbo_whistle.cpp',
  'src/effects/biquad_filter.cpp',
  'src/effects/distortion.cpp',
  'src/effects/reverb.cpp',
  'src/effects/effects_chain.cpp',
  'src/mixer/audio_mixer.cpp',
  'src/presets/preset_i4.cpp',
  'src/presets/preset_v8_cross.cpp',
  'src/presets/preset_v8_flat.cpp',
)

racing_sound_engine_lib = static_library('racing-sound-engine-core',
  core_sources,
  include_directories : inc_dirs,
  dependencies : platform_deps,
)

racing_sound_engine_dep = declare_dependency(
  link_with : racing_sound_engine_lib,
  include_directories : inc_dirs,
  dependencies : platform_deps,
)

# Demo executable
if get_option('build_demo')
  executable('engin-sound-demo',
    'src/main.cpp',
    dependencies : racing_sound_engine_dep,
  )
endif

# Tests
if get_option('build_tests')
  subdir('tests')
endif

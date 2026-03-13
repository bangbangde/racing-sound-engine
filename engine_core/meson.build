engine_core_inc = include_directories('include', 'src')

engine_core_sources = files(
  'src/synth_engine.cpp',
  'src/engin_core_api.cpp',
  'src/engine_model.cpp',
  'src/cylinder_bank.cpp',
  'src/harmonic_synth.cpp',
  'src/noise_generator.cpp',
  'src/sample_player.cpp',
  'src/turbo_whistle.cpp',
  'src/biquad_filter.cpp',
  'src/distortion.cpp',
  'src/reverb.cpp',
  'src/effects_chain.cpp',
  'src/audio_mixer.cpp',
)

engine_core_lib = static_library('engin-core',
  engine_core_sources,
  include_directories : engine_core_inc,
)

engine_core_dep = declare_dependency(
  link_with : engine_core_lib,
  include_directories : engine_core_inc,
)

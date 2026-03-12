test_sources = {
  'test_engine_model': files('test_engine_model.cpp'),
  'test_cylinder_bank': files('test_cylinder_bank.cpp'),
  'test_harmonic_synth': files('test_harmonic_synth.cpp'),
  'test_biquad_filter': files('test_biquad_filter.cpp'),
  'test_effects_chain': files('test_effects_chain.cpp'),
  'test_audio_mixer': files('test_audio_mixer.cpp'),
  'test_parameter_controller': files('test_parameter_controller.cpp'),
  'test_turbo_whistle': files('test_turbo_whistle.cpp'),
}

foreach name, src : test_sources
  exe = executable(name, src,
    dependencies : racing_sound_engine_dep,
  )
  test(name, exe)
endforeach

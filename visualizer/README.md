# Racing Sound Engine - Visualizer

Cross-platform real-time visualization layer for the racing sound engine.

## Architecture

```
┌─────────────────────────────────────────────┐
│              Unified JS API                  │
│   (TypeScript, platform-agnostic interface)  │
├──────────────┬──────────────────────────────┤
│  Node.js     │  Browser / WebAssembly       │
│  (N-API)     │  (Emscripten)                │
├──────────────┴──────────────────────────────┤
│         C API  (engin_core.h)               │
├─────────────────────────────────────────────┤
│         engine_core (pure DSP)              │
└─────────────────────────────────────────────┘
```

## Platform Targets

### Node.js Native Addon (N-API)
- Binds to `engine_core` static library via N-API (ABI-stable C addon API)
- Uses `node-gyp` or `cmake-js` for native compilation
- Targets: Windows, macOS, Linux
- Provides real-time audio processing in Node.js environment
- Can pair with Electron for desktop GUI

### Browser / WebAssembly (Emscripten)
- Compiles `engine_core` to WebAssembly via Emscripten
- Zero native dependencies (engine_core is pure math)
- Audio output via Web Audio API (`AudioWorkletProcessor`)
- Targets: All modern browsers (Chrome, Firefox, Safari, Edge)

## Unified JS API Design

```typescript
interface EnginCore {
  // Lifecycle
  create(sampleRate: number, bufferFrames: number): void;
  destroy(): void;

  // Configuration
  loadPreset(preset: EnginePreset): void;
  loadSample(slotIndex: number, pcmData: Float32Array,
             originalRate: number, loop: boolean): void;

  // Real-time control
  setParams(params: EngineParams): void;

  // Audio processing
  process(output: Float32Array): void;

  // State
  reset(): void;
}

interface EngineParams {
  rpm: number;
  throttle: number;     // 0.0 - 1.0
  gear: number;
  clutch: number;       // 0.0 - 1.0
  turboEnabled: boolean;
  presetId: number;
  shiftPhase: ShiftPhase;
}

interface EnginePreset {
  name: string;
  cylinderCount: number;
  firingOrder: number[];
  // ... maps 1:1 to C EnginePreset struct
}

enum ShiftPhase {
  None = 0,
  ClutchDisengage = 1,
  GearChange = 2,
  ClutchEngage = 3,
}
```

## C API Integration

The visualizer consumes `engin_core.h` exclusively:

| C Function | JS Method | Description |
|---|---|---|
| `engin_create()` | `create()` | Allocate engine instance |
| `engin_destroy()` | `destroy()` | Free engine instance |
| `engin_load_preset()` | `loadPreset()` | Load preset configuration |
| `engin_load_sample()` | `loadSample()` | Load PCM sample data |
| `engin_set_params()` | `setParams()` | Update real-time parameters |
| `engin_process()` | `process()` | Render audio frames |
| `engin_reset()` | `reset()` | Reset engine state |

## Visualization Features (Planned)

- Real-time RPM gauge / tachometer
- Waveform oscilloscope (time domain)
- Spectrum analyzer (FFT frequency domain)
- Per-cylinder impulse activity indicator
- Throttle / gear / turbo status display
- Preset switching UI

## Build Strategy

### Node.js Addon
```
engine_core (static lib)
    ↓ link
node_addon.cpp (N-API bindings)
    ↓ node-gyp build
engin_core.node (native addon)
```

### WebAssembly
```
engine_core sources
    ↓ emcc
engin_core.wasm + engin_core.js (glue)
    ↓ wrap
AudioWorkletProcessor (browser audio thread)
```

## Directory Structure (Planned)

```
visualizer/
├── README.md              ← this file
├── binding/
│   ├── napi/              ← Node.js N-API C++ wrapper
│   └── wasm/              ← Emscripten build scripts
├── src/
│   ├── index.ts           ← Unified JS API entry
│   ├── node-backend.ts    ← Node.js native implementation
│   ├── wasm-backend.ts    ← WASM implementation
│   └── worklet/
│       └── audio-processor.ts  ← AudioWorkletProcessor
├── ui/                    ← Visualization UI components
├── package.json
└── tsconfig.json
```

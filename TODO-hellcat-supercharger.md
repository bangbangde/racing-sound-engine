# TODO: Dodge Hellcat 声浪模拟 — 超充"猫叫"声

## 需求背景

Dodge Hellcat 搭载 6.2L 机械增压 HEMI V8（IHI 双螺杆超充，cross-plane 曲轴）。其标志性声音特征包括：

1. **超充啸叫 (Supercharger Whine)** — 双螺杆转子啮合产生的高频呜鸣，音高与 RPM 线性相关（皮带直驱，无迟滞）
2. **进气尖啸 ("猫叫")** — 高转速下进气歧管/超充入口的共振效应，频率随 RPM 上升而攀升，是 Hellcat 最具辨识度的声音
3. **深沉 V8 低吼** — cross-plane 不等间隔点火产生的经典美式 V8 节拍感
4. **泄压阀瞬态声** — 收油时旁通阀开启产生的"嗤"声

## 现有架构差距分析

| 能力 | 现状 | 差距 |
|------|------|------|
| V8 cross-plane 基础音色 | preset_v8_cross 已有 | 参数微调即可 |
| 超充啸叫 | TurboWhistle 模块为涡轮设计，单正弦+噪声，无转子谐波 | 需新建 SuperchargerSynth |
| 进气共振 (猫叫) | 单个静态 BandPass 滤波器，频率不随 RPM 变化 | 需扩展为 RPM-dependent 多共振模型 |
| 泄压阀声 | 无瞬态事件机制 | 可用现有 SamplePlayer 触发 |
| SamplePlayer RPM 跟踪 | pitch_ratio 在触发时固定，process() 不接收引擎状态 | 仅用于一次性事件，不做连续音源 |

## 决定采用的方案：程序化合成为主 + 采样补充细节

纯采样方案需要多层 RPM-keyed 交叉淡入淡出引擎，投入产出比差。程序化合成天然跟随 RPM，更适合此架构。

### 实施步骤

#### 步骤 1：新建 SuperchargerSynth 模块

- 位置：`src/synth/supercharger_synth.h/.cpp`
- 核心算法：多谐波加法合成
  - 基频 = RPM × 皮带轮比(pulley_ratio) × 叶瓣数(lobe_count) / 60
  - 叠加 3~5 阶整数谐波，模拟转子啮合频谱
  - 微量 FM 抖动模拟机械不稳定性
- 与引擎状态耦合：`process(output, frames, rpm, throttle, sample_rate)`
- 动态特性：音量与 RPM 正相关（非 RPM×throttle），无 spool 迟滞
- 参数：pulley_ratio, lobe_count, harmonic_count, harmonic_amplitudes[], max_amplitude, fm_jitter_amount

#### 步骤 2：扩展进气共振模型（模拟"猫叫"）

- 将 `intake_filter_`（单 BandPass）扩展为 2~3 个共振滤波器组
  - 参考 exhaust_resonances_[] 的现有架构
- 滤波器中心频率随 RPM 变化：
  - 例如：base_freq + rpm_norm × freq_range（低转约 400 Hz，高转约 1200 Hz）
- 在 `process_audio()` 中逐帧/逐 buffer 更新滤波器参数
- 需在 EnginePreset 中新增对应参数字段

#### 步骤 3：集成到 AudioEngine 流水线

- 在 AudioEngine 中添加 `SuperchargerSynth supercharger_synth_` 成员
- 分配 scratch buffer（scratch_supercharger_[]）和 mixer channel（channel 6）
- 在 process_audio() 中调用，传入 rpm/throttle
- 在 EnginePreset / load_preset() 中添加超充相关参数
- 新增 `supercharger_enabled` 开关（类似 turbo_enabled）

#### 步骤 4：创建 Hellcat 预设

- 位置：`src/presets/preset_hellcat.cpp`
- 基于 preset_v8_cross 调整：
  - 8 缸, cross-plane 点火序列
  - 谐波轮廓偏向中低频厚重感
  - 排气共振频率调低（更深沉）
  - 失真适度（drive ~2.5, mix ~0.4）
- 超充参数：pulley_ratio ≈ 2.36, lobe_count = 4（双螺杆典型值）
- 进气共振 RPM-dependent 参数调至高转啸叫区间

#### 步骤 5：用 SamplePlayer 补充瞬态细节

- 录制/获取以下一次性音效素材：
  - 旁通阀/泄压阀"嗤"声（收油触发）
  - 高转速回火/放炮声（overrun 触发）
- 通过现有 trigger() 接口在控制线程中按状态触发
- 需在控制逻辑中添加节流阀状态变化检测（throttle 从高→低时触发泄压声）

### 不在此方案范围内

- 多层 RPM-keyed 采样引擎（复杂度过高）
- 立体声输出（当前架构为 mono）
- SamplePlayer 的 RPM 跟踪改造（不用于连续音源）

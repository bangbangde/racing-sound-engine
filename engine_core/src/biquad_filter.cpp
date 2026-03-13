#include "biquad_filter.h"
#include "engin/constants.h"
#include <cmath>

namespace engin {

BiquadFilter::BiquadFilter() {
    reset();
}

void BiquadFilter::set_params(FilterType type, float freq, float Q, float gain_db,
                               SampleRate sample_rate) {
    float sr = static_cast<float>(sample_rate);
    float w0 = TWO_PI * freq / sr;
    float cos_w0 = std::cos(w0);
    float sin_w0 = std::sin(w0);
    float alpha = sin_w0 / (2.0f * Q);

    float a0 = 1.0f;
    float b0, b1, b2, a1, a2;

    switch (type) {
    case FilterType::LowPass:
        b0 = (1.0f - cos_w0) / 2.0f;
        b1 = 1.0f - cos_w0;
        b2 = (1.0f - cos_w0) / 2.0f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cos_w0;
        a2 = 1.0f - alpha;
        break;

    case FilterType::HighPass:
        b0 = (1.0f + cos_w0) / 2.0f;
        b1 = -(1.0f + cos_w0);
        b2 = (1.0f + cos_w0) / 2.0f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cos_w0;
        a2 = 1.0f - alpha;
        break;

    case FilterType::BandPass:
        b0 = alpha;
        b1 = 0.0f;
        b2 = -alpha;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cos_w0;
        a2 = 1.0f - alpha;
        break;

    case FilterType::Notch:
        b0 = 1.0f;
        b1 = -2.0f * cos_w0;
        b2 = 1.0f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cos_w0;
        a2 = 1.0f - alpha;
        break;

    case FilterType::Peak: {
        float A = std::pow(10.0f, gain_db / 40.0f);
        b0 = 1.0f + alpha * A;
        b1 = -2.0f * cos_w0;
        b2 = 1.0f - alpha * A;
        a0 = 1.0f + alpha / A;
        a1 = -2.0f * cos_w0;
        a2 = 1.0f - alpha / A;
        break;
    }

    case FilterType::LowShelf: {
        float A = std::pow(10.0f, gain_db / 40.0f);
        float sq = 2.0f * std::sqrt(A) * alpha;
        b0 = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 + sq);
        b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0);
        b2 = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 - sq);
        a0 = (A + 1.0f) + (A - 1.0f) * cos_w0 + sq;
        a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0);
        a2 = (A + 1.0f) + (A - 1.0f) * cos_w0 - sq;
        break;
    }

    case FilterType::HighShelf: {
        float A = std::pow(10.0f, gain_db / 40.0f);
        float sq = 2.0f * std::sqrt(A) * alpha;
        b0 = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + sq);
        b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0);
        b2 = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - sq);
        a0 = (A + 1.0f) - (A - 1.0f) * cos_w0 + sq;
        a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0);
        a2 = (A + 1.0f) - (A - 1.0f) * cos_w0 - sq;
        break;
    }
    }

    // Normalize
    b0_ = b0 / a0;
    b1_ = b1 / a0;
    b2_ = b2 / a0;
    a1_ = a1 / a0;
    a2_ = a2 / a0;
}

void BiquadFilter::process(Sample* buffer, FrameCount frames) {
    for (FrameCount i = 0; i < frames; ++i) {
        float x = buffer[i];
        float y = b0_ * x + z1_;
        z1_ = b1_ * x - a1_ * y + z2_;
        z2_ = b2_ * x - a2_ * y;
        buffer[i] = y;
    }
}

void BiquadFilter::reset() {
    z1_ = z2_ = 0.0f;
}

} // namespace engin

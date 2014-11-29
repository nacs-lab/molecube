#include <nacs-utils/utils.h>

#ifndef __NACS_PULSER_CONVERTER_H__
#define __NACS_PULSER_CONVERTER_H__

namespace NaCs {
namespace Pulser {

static NACS_INLINE constexpr double
c_sqr(double v)
{
    return v * v;
}

struct DDSConverter {
    static constexpr double pow2_2 = 1. / 4.;
    static constexpr double pow2_4 = c_sqr(pow2_2);
    static constexpr double pow2_8 = c_sqr(pow2_4);
    static constexpr double pow2_16 = c_sqr(pow2_8);
    static constexpr double pow2_32 = c_sqr(pow2_16);

    static NACS_INLINE constexpr double
    num2freq(uint32_t num, double clock)
    {
        return num * clock * pow2_32;
    }

    static NACS_INLINE constexpr uint32_t
    freq2num(double f, double clock)
    {
        return static_cast<uint32_t>(0.5 + f / clock * (1 / pow2_32));
    }

    static constexpr double phase_num = (1 << 14) / 90.0;

    static NACS_INLINE constexpr uint16_t
    phase2num(double phase)
    {
        return 0xFFFF & static_cast<uint16_t>(phase * phase_num + 0.5);
    }

    static NACS_INLINE constexpr double
    num2phase(uint16_t num)
    {
        return num * (1 / phase_num);
    }

    static NACS_INLINE constexpr uint32_t
    amp2num(double amp)
    {
        return 0x0FFF & static_cast<uint32_t>(amp * 4095.0 + 0.5);
    }

    static NACS_INLINE constexpr double
    num2amp(uint32_t num)
    {
        return num / 4095.0;
    }
};

}
}

#endif

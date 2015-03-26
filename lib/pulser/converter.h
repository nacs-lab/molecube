#include <nacs-utils/number.h>

#ifndef __NACS_PULSER_CONVERTER_H__
#define __NACS_PULSER_CONVERTER_H__

namespace NaCs {
namespace Pulser {

class DDSCvt {
    static constexpr double pow2_2 = 1. / 4.;
    static constexpr double pow2_4 = square(pow2_2);
    static constexpr double pow2_8 = square(pow2_4);
    static constexpr double pow2_16 = square(pow2_8);
    static constexpr double pow2_32 = square(pow2_16);

    static constexpr double phase_num = (1 << 14) / 90.0;
public:
    static inline constexpr double
    num2freq(uint32_t num, double clock)
    {
        return num * clock * pow2_32;
    }

    static inline constexpr uint32_t
    freq2num(double f, double clock)
    {
        return static_cast<uint32_t>(0.5 + f / clock * (1 / pow2_32));
    }

    static inline constexpr uint16_t
    phase2num(double phase)
    {
        return 0xffff & static_cast<uint16_t>(phase * phase_num + 0.5);
    }

    static inline constexpr double
    num2phase(uint32_t num)
    {
        return num * (1 / phase_num);
    }

    static inline constexpr uint32_t
    amp2num(double amp)
    {
        return 0x0fff & static_cast<uint32_t>(amp * 4095.0 + 0.5);
    }

    static inline constexpr double
    num2amp(uint32_t num)
    {
        return num / 4095.0;
    }
};

}
}

#endif

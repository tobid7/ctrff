#pragma once

#include <ctrff/pd_p_api.hpp>
#include <pd.hpp>

namespace ctrff {
namespace LZ11 {
CTRFF_API std::vector<PD::u8> Compress(const std::vector<PD::u8>& in);
}
}  // namespace ctrff
#include "Random.hpp"

#include <random>

namespace dc {

  RandomEngine
  makeRandomEngine(std::optional<uint32_t> seed)
  {
    if (seed.has_value()) {
      /*
	cv::RNG(0) is cv::RNG(), and two close seeds give two close states:
	spread the seed over the 64 bits of the state with an odd constant,
	so that seeds 0, 1, 2... give unrelated sequences and none of them is
	the default state.
      */
      constexpr uint64_t GOLDEN = 0x9E3779B97F4A7C15ULL;
      return RandomEngine((static_cast<uint64_t>(*seed) + 1) * GOLDEN);
    }

    //std::random_device gives 32 bits at a time
    std::random_device device;
    const uint64_t state =
      (static_cast<uint64_t>(device()) << 32) | static_cast<uint64_t>(device());

    return RandomEngine(state);
  }

} //namespace dc

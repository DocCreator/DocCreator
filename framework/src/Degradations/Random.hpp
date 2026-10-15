#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdint>
#include <limits>
#include <optional>

#include <opencv2/core/core.hpp>

#include <framework_global.h>

namespace dc {

  /**
    Random number generator used by the degradations.

    cv::RNG is used, and not a standard engine, because it can fill a whole
    cv::Mat with uniform or normal values at once (cf. cv::RNG::fill()).
    cv::randu() and cv::randn() do the same but on a generator shared by the
    whole thread (cf. cv::theRNG()), which a library has no business
    modifying, and which cannot be seeded by the caller.
  */
  using RandomEngine = cv::RNG;

  /**
    @param seed  seed of the generator.
    @return a generator seeded with @a seed, or seeded non-deterministically
    when @a seed is not provided.

    Two generators built with the same seed give the same sequence: pass a
    seed to get reproducible degradations.
  */
  extern FRAMEWORK_EXPORT
  RandomEngine
  makeRandomEngine(std::optional<uint32_t> seed = std::nullopt);

  /**
    Adapts a RandomEngine to the UniformRandomBitGenerator concept, so that it
    can be used with the algorithms of the standard library (std::shuffle()
    and others).
  */
  class RandomBitGenerator
  {
  public:
    using result_type = unsigned; ///< type of the values generated

    /** @param engine  generator to draw from. It must outlive this object. */
    explicit RandomBitGenerator(RandomEngine &engine)
      : m_engine(engine)
    {}

    /** @return the smallest value generated: 0. */
    static constexpr result_type min() { return 0; }
    /** @return the largest value generated. */
    static constexpr result_type max()
    {
      return std::numeric_limits<result_type>::max();
    }

    /** @return the next value of the engine. */
    result_type operator()() { return m_engine.next(); }

  private:
    RandomEngine &m_engine;
  };

} //namespace dc

#endif /* ! RANDOM_HPP */

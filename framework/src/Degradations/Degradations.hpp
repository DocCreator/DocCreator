#ifndef DEGRADATIONS_HPP
#define DEGRADATIONS_HPP

/**
   @file

   The degradations of the library, in one include.

   @section threads Threads

   Every degradation here is reentrant: it holds no state between calls and
   touches nothing another call can see. Several of them may thus be applied
   at the same time, on distinct images, from several threads; this is what
   the python module needs, where callers use threads and multiprocessing.

   What that rests on:

   - the generators are local objects, built by makeRandomEngine() for the
     duration of one call, and seeded by the caller (cf. Random.hpp). The
     library calls neither srand(), nor rand(), nor cv::theRNG(), whose
     state belongs to the whole program or to the whole thread;
   - no degradation holds a static or a namespace scope variable it writes
     to;
   - an image given to a degradation is only read, and the image it returns
     is its own.

   Applying a degradation to the *same* cv::Mat from several threads is
   another matter: a cv::Mat shares its pixels with its copies, so writing
   one from one thread while another reads it is the caller's race, not the
   library's.

   The one exception is Distortion3D: it drives a single OpenGL context,
   which GLFW requires be used by one thread at a time. Its four functions
   take a lock, so calling them concurrently is safe, but they do not run in
   parallel. Their header says so.

   tests/Degradations/testConcurrency.cpp applies each degradation from
   eight threads at once and compares every result with the one of a single
   call.
*/

/**
   @namespace dc

   The degradations of DocCreator, one namespace each, and what they share:
   the random number generators (Random.hpp), the validation of their
   arguments (Validate.hpp) and the listing of data directories
   (FileUtils.hpp).

   Unless its documentation says otherwise, a degradation takes an 8 bit
   image of 1, 3 or 4 channels (CV_8UC1, CV_8UC3 or CV_8UC4), throws
   std::invalid_argument for any other, leaves its input unchanged, and
   returns a new image of the same type and size. A degradation that draws
   random numbers takes an optional seed: two calls with the same seed and
   the same arguments give the same image.
*/

#include "BleedThrough.hpp"
#include "BlurFilter.hpp"
#include "ColorUtils.hpp"
#include "ConnectedComponent.hpp"
#include "Distortion3D.hpp"
#include "ElasticDeformation.hpp"
#include "GradientDomainDegradation.hpp"
#include "GrayscaleCharsDegradationModel.hpp"
#include "HoleDegradation.hpp"
#include "NoiseDegradation.hpp"
#include "PhantomCharacter.hpp"
#include "Random.hpp"
#include "RotationDegradation.hpp"
#include "ShadowBinding.hpp"

#endif /* ! DEGRADATIONS_HPP */

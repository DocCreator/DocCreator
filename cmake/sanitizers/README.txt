Building with a sanitiser
=========================

  cmake -S . -B buildasan -DBUILD_TESTING=ON -DSANITIZER=address+undefined
  make -C buildasan -j8
  cd buildasan && ctest

SANITIZER takes one of: none (the default), address, undefined,
address+undefined, thread, leak. AddressSanitizer and ThreadSanitizer
cannot be combined, so the option takes one value and not a list.

The degradation library alone configures and builds in about a minute,
which is the cheapest useful job:

  cmake -S . -B buildasan -DBUILD_ONLY_DEGRADATIONS=ON -DBUILD_TESTING=ON \
        -DSANITIZER=address+undefined

The files next to this one are suppression lists. They are not applied
automatically: a sanitiser reads them from its own environment variable,
which has to name an absolute path.

AddressSanitizer and UndefinedBehaviorSanitizer
-----------------------------------------------

  export ASAN_OPTIONS=detect_leaks=1:detect_stack_use_after_return=1:abort_on_error=0
  export LSAN_OPTIONS=suppressions=$PWD/cmake/sanitizers/lsan.supp
  export UBSAN_OPTIONS=suppressions=$PWD/cmake/sanitizers/ubsan.supp:print_stacktrace=1:halt_on_error=1

UBSan prints and goes on by default, which hides the first error in the
noise of the ones after it: halt_on_error=1 stops at the first. It also
stops at the reports of lzham, the codec of the .brs meshes, which reads
unaligned integers on purpose: ubsan.supp drops those, and the suite is
then quiet.

ThreadSanitizer
---------------

  export TSAN_OPTIONS=suppressions=$PWD/cmake/sanitizers/tsan.supp:second_deadlock_stack=1

ThreadSanitizer only sees the synchronisation of the code it
instrumented. The OpenCV libraries and the thread pool below them are not
rebuilt here, so cv::parallel_for_ hands the work over through
synchronisation TSan cannot see: the main thread fills a
cv::ParallelLoopBody, the pool passes it to a worker, and the worker
reading its own fields is reported as racing with the thread that filled
them. tsan.supp drops those, one entry per body, so that a body added
later is reported until somebody looks at it.

Without TSAN_OPTIONS the suppressions are not read and ctest fails on
reports that are all hand-offs. With it, the suite is quiet.

The test written for this job is tests/Degradations/testConcurrency.cpp,
which calls every exported degradation from eight threads at once.

A called_from_lib pattern has to match exactly one loaded library:
ThreadSanitizer stops at startup, before the first test, when one matches
two. That is why tsan.supp names libtbb.so and libtbbmalloc.so and not
libtbb.

Leaks
-----

LeakSanitizer comes with AddressSanitizer on linux
(ASAN_OPTIONS=detect_leaks=1, which is the default there), so
-DSANITIZER=leak is only useful to look for leaks without the rest of the
AddressSanitizer cost.

Qt allocates once and keeps what it allocates for the lifetime of the
process, so the GUI targets report leaks that are not ones; lsan.supp
drops the known ones.

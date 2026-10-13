Fuzz targets
============

Two readers take bytes they do not control and build an object from them:
the font readers (FontFileManager::readFont, which dispatches to the
binary .bof reader and the XML .of one) and the document XML loader
(XMLDocumentLoader). Every size, count and geometry of the result comes
from the file, and the sizes end up in allocations, which is where the
bugs those two readers have had came from. They are pure
bytes -> object functions, which is the shape libFuzzer wants.

Building
--------

libFuzzer is a clang feature; there is no gcc equivalent.

  cmake -S . -B buildfuzz -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_C_COMPILER=clang -DBUILD_FUZZERS=ON \
        -DSANITIZER=address+undefined
  make -C buildfuzz fuzz_font fuzz_xmldocument -j8

A fuzz target without a sanitiser only finds the crashes that happen to be
fatal, which misses most of the point: pass SANITIZER as above.

Running
-------

  cd buildfuzz/fuzz
  ./fuzz_font corpus_font -max_total_time=600
  ./fuzz_xmldocument corpus_xmldocument -max_total_time=600

The corpus directories are filled at configure time with the fonts of
data/font and the document of fuzz/corpus_xmldocument, so that a run
starts from something the readers accept. libFuzzer writes the inputs it
keeps back into them, so a later run starts where the last one stopped.

Useful options:

  -max_len=65536        a font or a document is never a megabyte
  -timeout=25           an input that takes longer is a finding of its own
  -rss_limit_mb=2048    so is one that asks for more memory than that
  -jobs=8 -workers=8    eight processes in parallel
  -runs=100000          a bounded run, for a CI job
  -only_ascii=1         for the XML target

A crash is written as crash-<sha1> in the working directory; the input is
replayed with

  ./fuzz_font crash-<sha1>

Minimising a crashing input:

  ./fuzz_font -minimize_crash=1 -runs=10000 crash-<sha1>

In CI
-----

A bounded run makes a usable job; it is not a substitute for a long one,
but it catches a regression on the corpus that has accumulated:

  ./fuzz_font corpus_font -runs=50000 -max_len=65536 -timeout=25

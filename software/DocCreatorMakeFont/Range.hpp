#ifndef RANGE_HPP
#define RANGE_HPP

static inline
int
hex2int(const char *s)
{
  return (int)strtol(s, NULL, 16);
}

struct Range
{
  Range() :
    start(-1), end(-1)
  {
  }
  
  Range(int rstart, int rend):
    start(rstart), end(rend)
  {
    assert(start <= end);
  }

  Range(const char* hexStart, const char *hexEnd):
    start(hex2int(hexStart)), end(hex2int(hexEnd))
  {
    assert(start <= end);
  }

  Range(const char* hexValue)
  {
    end = start = hex2int(hexValue);
  }
  
  int num() const
  {
    assert(start <= end);
    return end-start+1;
  }
  
  int start;
  int end;
};

typedef std::vector<Range> RangeVector;

#endif /* ! RANGE_HPP */

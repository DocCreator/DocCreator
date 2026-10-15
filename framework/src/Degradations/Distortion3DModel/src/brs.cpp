#include "brs.hpp"

#include <algorithm>
#include <cassert>
#include <cstring> //memcpy
#include <fstream>
#include <new> //bad_alloc
#include <iostream> //DEBUG
#include <vector>

#include "Mesh.hpp"

//Comment/Uncomment the following line to disable/enable index buffer compression
#define USE_INDEXBUFFERCOMPRESSION 1

//Comment/Uncomment the following line to disable/enable lzham compression
#define USE_LZHAM

#ifdef USE_INDEXBUFFERCOMPRESSION
#include "IndexBufferCompression/indexbuffercompression.h"
#include "IndexBufferCompression/indexbufferdecompression.h"
#endif //USE_INDEXBUFFERCOMPRESSION

#ifdef USE_LZHAM

// Define LZHAM_DEFINE_ZLIB_API causes lzham.h to remap the standard zlib.h
// functions/macro definitions to lzham's.
#define LZHAM_DEFINE_ZLIB_API
#include "lzham_static_lib.h"

#ifdef LZHAM_64BIT
#define LZHAMTEST_MAX_POSSIBLE_DICT_SIZE LZHAM_MAX_DICT_SIZE_LOG2_X64
// 256MB default dictionary size under x64 (max is 512MB, but this requires more
// than 4GB of physical memory without thrashing)
#define LZHAMTEST_DEFAULT_DICT_SIZE 28
#else
#define LZHAMTEST_MAX_POSSIBLE_DICT_SIZE LZHAM_MAX_DICT_SIZE_LOG2_X86
// 64MB default dictionary size under x86
#define LZHAMTEST_DEFAULT_DICT_SIZE LZHAM_MAX_DICT_SIZE_LOG2_X86
#endif

//"Huffman table update frequency. 0=Internal def, Def=8, higher=faster.
// Lower settings=slower decompression, but higher ratio. Note 1=impractically
// slow."
#define MY_LZHAM_TABLE_UPDATE_RATE 4 //LZHAM_DEFAULT_TABLE_UPDATE_RATE

//LZHAM can write buffer of size at most LZHAM_maxSize
static const size_t LZHAM_maxSize = UINT32_MAX;

#endif //USE_LZHAM

enum BRSType { VERTICES = 1, TEXCOORDS = 2, NORMALS = 4 };

const int HEADER = ('B' << 24 | 'R' << 16 | 'S' << 8);
const int HEADER_MASK = 0xFFFFFF00;
const int TYPE_MASK = 0x000000FF;

bool
readHeader(std::ifstream &in, BRSType &type)
{
  uint32_t t;
  in.read((char *)&t, sizeof(t));
  if (!in) { //B: in.bad() is not set by a short read
    return false;
  }

  if ((t & HEADER_MASK) != HEADER) {
    return false;
  }

  //B: checked as an integer before the cast: casting a value that is not
  // one of BRSType's to it is undefined
  const uint32_t typeValue = (t & TYPE_MASK);
  if (typeValue != VERTICES
      && typeValue != (VERTICES | TEXCOORDS)
      && typeValue != (VERTICES | NORMALS)
      && typeValue != (VERTICES | TEXCOORDS | NORMALS)) {
    return false;
  }
  type = static_cast<BRSType>(typeValue);

  return true;
}

bool
isBRSFile(const std::string &filename)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (!in) {
    std::cerr<<"ERROR: unable to open: "<<filename<<"\n";
    return false;
  }
  BRSType type;
  return readHeader(in, type);
}

#ifdef USE_LZHAM

/*
  Write buffer @a data of size @a szb to ouput stream @a out compressed with LZHAM.
  It takes into account max size of buffer that LZHAM can handle and write in chunks if necessary.

  @param[in] szb  size in Bytes to write

 */
bool
writeLZHAM(std::ostream &out, const char *data, size_t szb)
{
  //from lzham.h
  // IMPORTANT: The values of m_dict_size_log2, m_table_update_rate,
  //  m_table_max_update_interval, and m_table_update_interval_slow_rate MUST
  // match during compression and decompression. The codec does not verify these
  // values for you,
  // if you don't use the same settings during decompression it will fail
  // (usually with a LZHAM_DECOMP_STATUS_FAILED_BAD_CODE error).

  //table_update_rate : Huffman table update frequency. 0=Internal def, Def=8, higher=faster.
  //        Lower settings=slower decompression, but higher ratio. Note
  //        1=impractically slow.

  lzham_compress_params params;
  memset(&params, 0, sizeof(params));
  params.m_struct_size = sizeof(lzham_compress_params);
  params.m_dict_size_log2 = LZHAMTEST_DEFAULT_DICT_SIZE;
  params.m_max_helper_threads = -1; // -1 try to autodetect
  params.m_level =
    LZHAM_COMP_LEVEL_UBER; //LZHAM_COMP_LEVEL_FASTER; //LZHAM_COMP_LEVEL_DEFAULT; //
  params.m_compress_flags |= LZHAM_COMP_FLAG_EXTREME_PARSING;
  params.m_compress_flags |= LZHAM_COMP_FLAG_DETERMINISTIC_PARSING;
  params.m_table_update_rate = MY_LZHAM_TABLE_UPDATE_RATE;

  const lzham_uint32 dict_size_log2 = params.m_dict_size_log2;
  const lzham_uint32 table_update_rate = params.m_table_update_rate;

  //B:REM: we save dict_size_log2 & table_update_rate at each call
  // If this function is called several times, it may takes disk space...
  out.write((const char *)&(dict_size_log2), sizeof(dict_size_log2));
  out.write((const char *)&(table_update_rate), sizeof(table_update_rate));

  //WARNING: lzham_compress_memory() can write at most a buffer of size UINT32_MAX !

  size_t toWriteSize = szb;
  size_t shift = 0;
  void *dst = nullptr;
  size_t dstSize = 0;

  while (toWriteSize > 0) {

    lzham_uint32 szw =
      (toWriteSize < LZHAM_maxSize) ? toWriteSize : LZHAM_maxSize;

    const size_t worstCaseSize = lzham_z_compressBound(szw);

    if (dstSize < worstCaseSize) {
      free(dst);
      dstSize = worstCaseSize;
      dst = malloc(dstSize);
      if (dst == nullptr)
	return false;
    }

    size_t out_num_bytes = worstCaseSize;
    lzham_compress_status_t status =
      lzham_compress_memory(&params,
                            static_cast<lzham_uint8 *>(dst),
                            &out_num_bytes,
                            (const lzham_uint8 *)(data + shift),
                            szw,
                            nullptr);
    if (status != LZHAM_COMP_STATUS_SUCCESS || out_num_bytes == 0u) {
      std::cerr << "Error: unable to compress buffer in LZHAM format\n";
      std::cerr << "status=" << status << " =?= LZHAM_COMP_STATUS_SUCCESS="
                << LZHAM_COMP_STATUS_SUCCESS << "\n";
      std::cerr << "LZHAM_COMP_STATUS_NOT_FINISHED="
                << LZHAM_COMP_STATUS_NOT_FINISHED << "\n";
      std::cerr << "LZHAM_COMP_STATUS_NEEDS_MORE_INPUT="
                << LZHAM_COMP_STATUS_NEEDS_MORE_INPUT << "\n";
      std::cerr << "LZHAM_COMP_STATUS_FAILED=" << LZHAM_COMP_STATUS_FAILED
                << "\n";
      std::cerr << "LZHAM_COMP_STATUS_FAILED_INITIALIZING="
                << LZHAM_COMP_STATUS_FAILED_INITIALIZING << "\n";
      std::cerr << "LZHAM_COMP_STATUS_INVALID_PARAMETER="
                << LZHAM_COMP_STATUS_INVALID_PARAMETER << "\n";
      std::cerr << "sizeToCompress=" << szw
                << " worstCaseSize=" << worstCaseSize
                << " out_num_bytes=" << out_num_bytes << "\n";
      free(dst);
      return false;
    }

    out.write((const char *)&out_num_bytes, sizeof(out_num_bytes));

    //std::cerr<<"writeC "<<out_num_bytes<<"\n";

    out.write((const char *)dst, out_num_bytes);

    assert(szw <= toWriteSize);
    toWriteSize -= szw;
    shift += szw;
  }

  free(dst);

  return true;
}

/*
  @return the number of bytes left to read in @a in, or 0 if it cannot
  tell. A size read from a file cannot be larger than that.
*/
static uint64_t
bytesLeft(std::istream &in)
{
  const std::streampos current = in.tellg();
  if (!in || current < 0)
    return 0;
  in.seekg(0, std::ios::end);
  const std::streampos end = in.tellg();
  in.seekg(current);
  if (!in || end < current)
    return 0;
  return static_cast<uint64_t>(end - current);
}

/*
  Read buffer @a data of size @a szb from input stream @a in compressed with LZHAM.

  @param[in] szb  size in Bytes to read

 */
bool
readLZHAM(std::istream &in, char *data, size_t szb)
{

  //B: the reads are checked with !in: in.bad() is not set by a short
  // read, which left the values read uninitialised
  lzham_uint32 dict_size_log2 = 0;
  in.read((char *)&dict_size_log2, sizeof(dict_size_log2));
  if (!in)
    return false;

  lzham_uint32 table_update_rate = 0;
  in.read((char *)&table_update_rate, sizeof(table_update_rate));
  if (!in)
    return false;

  lzham_decompress_params params;
  memset(&params, 0, sizeof(params));
  params.m_struct_size = sizeof(lzham_decompress_params);
  params.m_dict_size_log2 = dict_size_log2;
  params.m_table_update_rate = table_update_rate; //MY_LZHAM_TABLE_UPDATE_RATE

  //std::cerr<<"read dict_size_log2="<<dict_size_log2<<" table_update_rate="<<table_update_rate<<"\n";

  size_t toReadSize = szb;
  size_t shift = 0;
  void *src = nullptr;
  size_t srcSize = 0;

  while (toReadSize > 0) {

    lzham_uint32 szw =
      (toReadSize < LZHAM_maxSize) ? toReadSize : LZHAM_maxSize;

    if (shift + szw > szb) {
      std::cerr << "LZHAM: decompression failed (not enough space in buffer)\n";
      free(src);
      return false;
    }

    size_t comp_size = 0;
    in.read((char *)&comp_size, sizeof(comp_size));
    //std::cerr<<"readC "<<comp_size<<"\n";
    //B: and checked against the file before it is allocated
    if (!in || comp_size == 0 || comp_size > bytesLeft(in)) {
      free(src);
      return false;
    }

    if (srcSize < comp_size) {
      free(src);
      src = nullptr;
      srcSize = comp_size;
      src = malloc(srcSize);
      if (src == nullptr)
	return false;
    }

    in.read((char *)src, comp_size);
    if (!in) {
      std::cerr << "LZHAM: failed to read compressed data\n";
      free(src);
      return false;
    }

    size_t uncomp_size = szw;
    lzham_decompress_status_t status =
      lzham_decompress_memory(&params,
                              (lzham_uint8 *)(data + shift),
                              &uncomp_size,
                              static_cast<const lzham_uint8 *>(src),
                              comp_size,
                              nullptr);
    if (status != LZHAM_DECOMP_STATUS_SUCCESS) {
      std::cerr << "LZHAM: decompression failed\n";
      std::cerr << "uncomp_size=" << uncomp_size << "\n";
      std::cerr << "status=" << status << " =?= LZHAM_DECOMP_STATUS_SUCCESS="
                << LZHAM_DECOMP_STATUS_SUCCESS << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_NOT_FINISHED="
                << LZHAM_DECOMP_STATUS_NOT_FINISHED << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_HAS_MORE_OUTPUT="
                << LZHAM_DECOMP_STATUS_HAS_MORE_OUTPUT << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_NEEDS_MORE_INPUT="
                << LZHAM_DECOMP_STATUS_NEEDS_MORE_INPUT << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FIRST_SUCCESS_OR_FAILURE_CODE="
                << LZHAM_DECOMP_STATUS_FIRST_SUCCESS_OR_FAILURE_CODE << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FIRST_FAILURE_CODE="
                << LZHAM_DECOMP_STATUS_FIRST_FAILURE_CODE << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_INITIALIZING="
                << LZHAM_DECOMP_STATUS_FAILED_INITIALIZING << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_DEST_BUF_TOO_SMALL="
                << LZHAM_DECOMP_STATUS_FAILED_DEST_BUF_TOO_SMALL << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_EXPECTED_MORE_RAW_BYTES="
                << LZHAM_DECOMP_STATUS_FAILED_EXPECTED_MORE_RAW_BYTES << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_BAD_CODE="
                << LZHAM_DECOMP_STATUS_FAILED_BAD_CODE << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_ADLER32="
                << LZHAM_DECOMP_STATUS_FAILED_ADLER32 << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_BAD_RAW_BLOCK="
                << LZHAM_DECOMP_STATUS_FAILED_BAD_RAW_BLOCK << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_BAD_COMP_BLOCK_SYNC_CHECK="
                << LZHAM_DECOMP_STATUS_FAILED_BAD_COMP_BLOCK_SYNC_CHECK << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_BAD_ZLIB_HEADER="
                << LZHAM_DECOMP_STATUS_FAILED_BAD_ZLIB_HEADER << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_NEED_SEED_BYTES="
                << LZHAM_DECOMP_STATUS_FAILED_NEED_SEED_BYTES << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_BAD_SEED_BYTES="
                << LZHAM_DECOMP_STATUS_FAILED_BAD_SEED_BYTES << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_FAILED_BAD_SYNC_BLOCK="
                << LZHAM_DECOMP_STATUS_FAILED_BAD_SYNC_BLOCK << "\n";
      std::cerr << "LZHAM_DECOMP_STATUS_INVALID_PARAMETER="
                << LZHAM_DECOMP_STATUS_INVALID_PARAMETER << "\n";

      free(src);
      return false;
    }

    //B: a chunk that decompresses to nothing would loop for ever
    if (uncomp_size == 0 || uncomp_size > toReadSize) {
      free(src);
      return false;
    }
    shift += uncomp_size;
    toReadSize -= uncomp_size;
  }

  free(src);

  return true;
}

#endif //USE_LZHAM

#ifndef USE_LZHAM

//without LZHAM decompression

bool
readBRS(const std::string &filename, Mesh &mesh)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (!in) {
    return false;
  }
  BRSType type;
  bool readHeaderOk = readHeader(in, type);
  if (!readHeaderOk)
    return false;

  uint32_t numVertices = 0;
  in.read((char *)&numVertices, sizeof(numVertices));
  if (in.bad())
    return false;
  uint32_t numTris = 0;
  in.read((char *)&numTris, sizeof(numTris));
  if (in.bad())
    return false;
  if (numVertices == 0 || numTris == 0)
    return false;

  {
    mesh.allocateTriangles(numTris);

#ifndef USE_INDEXBUFFERCOMPRESSION
    const size_t s = mesh.numTriangles() * 3 * sizeof(uint32_t);
    in.read((char *)mesh.triangles, s);
#else
    {
      size_t size = 0;
      in.read((char *)&size, sizeof(size));
      size_t size8 = (size + 7) & (~7); //must be a multiple of 8 bytes.
      uint8_t *data = (uint8_t *)malloc(size8);
      if (data == nullptr)
	return false;
      in.read((char *)data, size);
      if (in.bad()) {
        mesh.clear();
        return false;
      }
#ifndef NDEBUG
      std::cerr << "read: size=" << size << " size8=" << size8
                << " mesh.numTriangles()*3*sizeof(uint32_t)="
                << mesh.numTriangles() * 3 * sizeof(uint32_t) << "\n";
#endif
      ReadBitstream rb(data, size8);
      DecompressIndexBuffer(mesh.triangles.data(), mesh.numTriangles(), rb);
      free(data);

#ifndef NDEBUG
      {
        std::cerr << "READ mesh.triangles: ";
        //B: was 21 indices whatever the mesh holds
        for (size_t i = 0; i < std::min<size_t>(21, mesh.triangles.size()); ++i)
          std::cerr << mesh.triangles[i] << " ";
        std::cerr << "\n";
      }
#endif //NDEBUG
    }
#endif

    if (in.bad()) {
      mesh.clear();
      return false;
    }
  }
  assert(mesh.numTriangles() == numTris);

  assert(type & VERTICES);
  {
    mesh.allocateVertices(numVertices);

    const size_t s = mesh.numVertices() * 3 * sizeof(float);

    in.read((char *)mesh.vertices, s);
    if (in.bad()) {
      mesh.clear();
      return false;
    }
  }
  assert(mesh.numVertices() == numVertices);

  if (type & TEXCOORDS) {
    mesh.allocateTexCoords();

    const size_t s = mesh.numVertices() * 2 * sizeof(float);

    in.read((char *)mesh.texCoords, s);
    if (in.bad()) {
      mesh.clear();
      return false;
    }

    assert(mesh.hasTexCoords());
  }

  if (type & NORMALS) {
    mesh.allocateNormals();

    const size_t s = mesh.numVertices() * 3 * sizeof(float);

    in.read((char *)mesh.normals, s);
    if (in.bad()) {
      mesh.clear();
      return false;
    }

    assert(mesh.hasNormals());
  }

  assert(mesh.isValid());

  return true;
}

#else //USE_LZHAM

//use LZHAM compression

/*
  B: readBRS() trusted every count and size of the file. A crafted or
  corrupt mesh could:
   - give an index buffer size near SIZE_MAX, which (size + 7) & ~7
     wrapped to 0: 8 bytes were allocated and @a size copied into them;
   - give an index buffer, a vertex count or a triangle count that do not
     match the decompressed data, which was then read past its end by the
     memcpy() of the vertices, or by the index decompressor, whose bit
     reader did no bounds checking;
   - decode triangle indices past the vertices, for whoever draws the mesh;
   - be truncated: the reads were checked with in.bad(), which a short
     read does not set (it sets failbit and eofbit).
  The decompressed buffer was also never freed.

  Every size is now checked against the layout writeBRS() produces --
  [size_t: index data size][index data][vertices][texcoords][normals] --
  before it is used, the bit reader cannot read past its buffer and says
  when the stream asked for more than it holds, every index is checked,
  every read is checked with !in, and the buffers are vectors. A file
  that fails any check gives false and an empty @a mesh.
*/
namespace {

  template <typename T>
  bool
  readValue(std::istream &in, T &value)
  {
    in.read(reinterpret_cast<char *>(&value), sizeof(value));
    return static_cast<bool>(in);
  }

} //anonymous namespace

static bool
readBRSAux(std::ifstream &in, Mesh &mesh)
{
  BRSType type;
  if (!readHeader(in, type))
    return false;

  uint32_t numVertices = 0;
  uint32_t numTris = 0;
  if (!readValue(in, numVertices) || !readValue(in, numTris))
    return false;
  if (numVertices == 0 || numTris == 0)
    return false;

  size_t totalSize = 0;
  if (!readValue(in, totalSize))
    return false;

  //what the counts imply, in 64 bits: none of this can overflow
  const uint64_t sv3 = uint64_t(numVertices) * 3 * sizeof(float);
  const uint64_t sv2 = uint64_t(numVertices) * 2 * sizeof(float);
  const uint64_t verticesSize =
    sv3 + ((type & TEXCOORDS) ? sv2 : 0) + ((type & NORMALS) ? sv3 : 0);
  //a compressed index or triangle is a code of a few bits and at most a
  // 5 byte variable length int per index: 32 bytes a triangle is a wide
  // upper bound. It only keeps the allocation below in proportion to the
  // counts; decodeBRSData() checks the layout exactly.
  const uint64_t maxIndicesSize = uint64_t(numTris) * 32 + 64;
  if (totalSize < sizeof(size_t) + verticesSize
      || totalSize > sizeof(size_t) + maxIndicesSize + verticesSize)
    return false;

  //The counts can be 2^32-1 each, so the bound above still allows a
  // terabyte. The data is compressed in what is left of the file, and
  // compression does not multiply data without bound: the meshes of
  // data/Mesh expand 2 to 12 times. 1024 times is far above that, and caps
  // what a small file can make this allocate.
  const uint64_t MAX_EXPANSION = 1024;
  if (totalSize > bytesLeft(in) * MAX_EXPANSION)
    return false;

  std::vector<unsigned char> buffer(totalSize);

#ifdef USE_LZHAM_HIGHLEVEL

  //Warning: we do not check buffer size ! We consider that we only have one chunk of data

  uLong cmp_len = 0;
  lzham_uint32 dict_size_log2 = 0;
  lzham_uint32 table_update_rate = 0;
  if (!readValue(in, cmp_len) || !readValue(in, dict_size_log2)
      || !readValue(in, table_update_rate))
    return false;

  std::vector<unsigned char> compressed(cmp_len);
  in.read(reinterpret_cast<char *>(compressed.data()), cmp_len);
  if (!in)
    return false;

  uLong uncomp_len = totalSize;
  const int cmp_status =
    uncompress(buffer.data(), &uncomp_len, compressed.data(), cmp_len);
  if (cmp_status != Z_OK || totalSize != uncomp_len)
    return false;

#else
  //Use low level functions of LZHAM
  // (it seems we are able to write more compressed data, see writeBRS()).

  if (!readLZHAM(in, reinterpret_cast<char *>(buffer.data()), totalSize))
    return false;

#endif //USE_LZHAM_HIGHLEVEL

  return decodeBRSData(type, numVertices, numTris, buffer.data(), totalSize, mesh);
}

#endif //USE_LZHAM

static bool
decodeBRSDataAux(int type,
              uint32_t numVertices,
              uint32_t numTris,
              const unsigned char *data,
              size_t totalSize,
              Mesh &mesh)
{
  mesh.clear();
  if (numVertices == 0 || numTris == 0 || data == nullptr)
    return false;
  if (type != VERTICES
      && type != (VERTICES | TEXCOORDS)
      && type != (VERTICES | NORMALS)
      && type != (VERTICES | TEXCOORDS | NORMALS))
    return false;

  const uint64_t sv3 = uint64_t(numVertices) * 3 * sizeof(float);
  const uint64_t sv2 = uint64_t(numVertices) * 2 * sizeof(float);
  const uint64_t verticesSize =
    sv3 + ((type & TEXCOORDS) ? sv2 : 0) + ((type & NORMALS) ? sv3 : 0);

  const unsigned char *cBUF = data;

#ifndef USE_INDEXBUFFERCOMPRESSION
  {
    const uint64_t st3 = uint64_t(numTris) * 3 * sizeof(int32_t);
    if (totalSize != st3 + verticesSize)
      return false;

    mesh.allocateTriangles(numTris);
    memcpy(mesh.triangles.data(), cBUF, st3);
    cBUF += st3;
  }
#else
  {
    if (totalSize < sizeof(size_t) + verticesSize)
      return false;

    size_t size = 0;
    memcpy(&size, cBUF, sizeof(size));
    cBUF += sizeof(size);

    //the index data is what is left before the vertices
    if (size != totalSize - sizeof(size_t) - verticesSize)
      return false;
    //a triangle takes at least one bit: this caps the allocation below by
    // the data, rather than by a count a file can set to 2^32-1
    if (uint64_t(numTris) > uint64_t(size) * 8)
      return false;

    mesh.allocateTriangles(numTris);

    //cf readbitstream.h: the reader stops at the end of the index data,
    // and says if the decoder asked for more
    ReadBitstream rb(cBUF, size);
    DecompressIndexBuffer(mesh.triangles.data(), mesh.numTriangles(), rb);
    if (rb.Overrun())
      return false;
    cBUF += size;
  }
#endif //USE_INDEXBUFFERCOMPRESSION
  assert(mesh.numTriangles() == numTris);

  for (const uint32_t index : mesh.triangles) {
    if (index >= numVertices)
      return false;
  }

  mesh.allocateVertices(numVertices);
  memcpy(mesh.vertices.data(), cBUF, sv3);
  cBUF += sv3;

  if ((type & TEXCOORDS) != 0) {
    mesh.allocateTexCoords();
    memcpy(mesh.texCoords.data(), cBUF, sv2);
    cBUF += sv2;
  }

  if ((type & NORMALS) != 0) {
    mesh.allocateNormals();
    memcpy(mesh.normals.data(), cBUF, sv3);
    cBUF += sv3;
  }

  assert(static_cast<size_t>(cBUF - data) == totalSize);
  assert(mesh.isValid());

  return true;
}

bool
decodeBRSData(int type,
              uint32_t numVertices,
              uint32_t numTris,
              const unsigned char *data,
              size_t totalSize,
              Mesh &mesh)
{
  bool ok = false;
  try {
    ok = decodeBRSDataAux(type, numVertices, numTris, data, totalSize, mesh);
  }
  catch (const std::bad_alloc &) {
    ok = false;
  }
  if (!ok)
    mesh.clear();
  return ok;
}

#ifdef USE_LZHAM

bool
readBRS(const std::string &filename, Mesh &mesh)
{
  mesh.clear();

  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (!in) {
    return false;
  }

  bool ok = false;
  try {
    ok = readBRSAux(in, mesh);
  }
  catch (const std::bad_alloc &) {
    //counts that are consistent, but too large for this machine
    ok = false;
  }
  if (!ok) {
    mesh.clear();
  }
  return ok;
}

#endif //USE_LZHAM

  /////////////////////////////////////////////////////////////

#ifndef USE_LZHAM

//without using LZHAM

bool
writeBRS(const std::string &filename, const Mesh &mesh)
{
  if (!mesh.isValid())
    return false;

  std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
  if (!out) {
    return false;
  }

  const BRSType type =
    (BRSType)(VERTICES | (mesh.hasTexCoords() ? TEXCOORDS : VERTICES) |
              (mesh.hasNormals() ? NORMALS : VERTICES));

  const uint32_t header = HEADER | type;

  out.write((const char *)&header, sizeof(uint32_t));
  if (out.bad())
    return false;

  const uint32_t numVertices = mesh.numVertices();
  out.write((const char *)&numVertices, sizeof(uint32_t));
  if (out.bad())
    return false;
  const uint32_t numTriangles = mesh.numTriangles();
  out.write((const char *)&numTriangles, sizeof(uint32_t));
  if (out.bad())
    return false;

  {
#ifndef USE_INDEXBUFFERCOMPRESSION
    const size_t s = mesh.numTriangles() * 3 * sizeof(float);
    out.write((const char *)mesh.triangles, s);

    {
      const size_t s = mesh.numVertices() * 3 * sizeof(float);
      out.write((const char *)mesh.vertices, s);
      if (out.bad())
        return false;
    }

    if (mesh.hasTexCoords()) {
      const size_t s = mesh.numVertices() * 2 * sizeof(float);
      out.write((const char *)mesh.texCoords, s);
      if (out.bad())
        return false;
    }

    if (mesh.hasNormals()) {
      const size_t s = mesh.numVertices() * 3 * sizeof(float);
      out.write((const char *)mesh.normals, s);
      if (out.bad())
        return false;
    }

#else
    {
#ifndef NDEBUG
      {
        std::cerr << "SAVE mesh.triangles: ";
        //B: was 21 indices whatever the mesh holds
        for (size_t i = 0; i < std::min<size_t>(21, mesh.triangles.size()); ++i)
          std::cerr << mesh.triangles[i] << " ";
        std::cerr << "\n";
      }
#endif //NDEBUG

      const size_t initialBufferCapacity =
        4096; //arbitrary //should be a multiple of 8 and > 0
      WriteBitstream wb(initialBufferCapacity);
      const IndexBufferCompressionFormat format =
        IBCF_PER_TRIANGLE_PREFIX_ENTROPY; //IBCF_PER_TRIANGLE_1; //IBCF_AUTO;
      uint32_t *vertexRemap =
        (uint32_t *)malloc(mesh.numVertices() * sizeof(uint32_t));
      if (vertexRemap == nullptr) {
	return false;
      }
      CompressIndexBuffer(mesh.triangles.data(),
                          mesh.numTriangles(),
                          vertexRemap,
                          mesh.numVertices(),
                          format,
                          wb);
      wb.Finish();

#ifndef NDEBUG
      {
        uint32_t c = 0;
        uint32_t r = 0;
        for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
          c += (vertexRemap[i] == 0xFFFFFFFF);
          r += (vertexRemap[i] != i);
        }
        std::cerr << "number of unused vertices=" << c << "/"
                  << mesh.numVertices() << "\n";
        std::cerr << "number of remapped vertices=" << r << "/"
                  << mesh.numVertices() << "\n";
        int remap = 0;
        for (uint32_t i = 0; i < mesh.numVertices() && remap < 12; ++i) {
          if (i != vertexRemap[i]) {
            std::cerr << i << " -> " << vertexRemap[i] << "\n";
            ++remap;
          }
        }
      }
#endif //NDEBUG

      //save compressed triangles indices
      size_t size = wb.ByteSize();
      out.write((const char *)&size, sizeof(size));
      if (out.bad())
        return false;
      out.write((const char *)wb.RawData(), size);
#ifndef NDEBUG
      std::cerr << "original size  =" << mesh.numTriangles() * 3 * sizeof(float)
                << "\n";
      std::cerr << "compressed size=" << size << "\n";
#endif //NDEBUG

      //save vertices (with remap)
      {
        const size_t nv3 = mesh.numVertices() * 3;
        std::vector<float> v(nv3);

        for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < v.size() / 3);
            assert(oldInd < mesh.numVertices());
            v[3 * newInd + 0] = mesh.vertices[3 * oldInd + 0];
            v[3 * newInd + 1] = mesh.vertices[3 * oldInd + 1];
            v[3 * newInd + 2] = mesh.vertices[3 * oldInd + 2];
          }
        }
        const size_t s = nv3 * sizeof(float);
        out.write((const char *)v.data(), s);
        if (out.bad())
          return false;
      }

      //save texcoords if any (with remap)
      if (mesh.hasTexCoords()) {
        const size_t nv2 = mesh.numVertices() * 2;
        std::vector<float> v(nv2);

        for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < v.size() / 2);
            assert(oldInd < mesh.numVertices());
            v[2 * newInd + 0] = mesh.texCoords[2 * oldInd + 0];
            v[2 * newInd + 1] = mesh.texCoords[2 * oldInd + 1];
          }
        }
        const size_t s = nv2 * sizeof(float);
        out.write((const char *)v.data(), s);
        if (out.bad())
          return false;
      }

      //save normals if any (with remap)
      if (mesh.hasNormals()) {
        const size_t nv3 = mesh.numVertices() * 3;
        std::vector<float> v(nv3);

        for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < v.size() / 3);
            assert(oldInd < mesh.numVertices());
            v[3 * newInd + 0] = mesh.normals[3 * oldInd + 0];
            v[3 * newInd + 1] = mesh.normals[3 * oldInd + 1];
            v[3 * newInd + 2] = mesh.normals[3 * oldInd + 2];
          }
        }
        const size_t s = nv3 * sizeof(float);
        out.write((const char *)v.data(), s);
        if (out.bad())
          return false;
      }

      /*
      //remap
      {
        const size_t nv3 = mesh.numVertices()*3;
        std::vector<float> v(nv3);

        for (size_t i=0; i<nv3; ++i) {
          v[i] = mesh.vertices[i];
        }

        for (uint32_t i=0; i<mesh.numVertices(); ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < mesh.numVertices());
            assert(oldInd < v.size()/3);
            mesh.vertices[3*newInd+0] = v[3*oldInd+0];
            mesh.vertices[3*newInd+1] = v[3*oldInd+1];
            mesh.vertices[3*newInd+2] = v[3*oldInd+2];
          }
        }

        if (mesh.hasNormals()) {
          for (size_t i=0; i<nv3; ++i) {
            assert(i < v.size());
            v[i] = mesh.normals[i];
          }

          for (uint32_t i=0; i<mesh.numVertices(); ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices());
              assert(oldInd < v.size()/3);
              mesh.normals[3*newInd+0] = v[3*oldInd+0];
              mesh.normals[3*newInd+1] = v[3*oldInd+1];
              mesh.normals[3*newInd+2] = v[3*oldInd+2];
            }
          }

        }

        if (mesh.hasTexCoords()) {
          const size_t nv2 = mesh.numVertices()*2;
          v.resize(nv2);
          for (size_t i=0; i<nv2; ++i) {
            assert(i < v.size());
            v[i] = mesh.normals[i];
          }

          for (uint32_t i=0; i<mesh.numVertices(); ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices());
              assert(oldInd < v.size()/2);
              mesh.texCoords[2*newInd+0] = v[2*oldInd+0];
              mesh.texCoords[2*newInd+1] = v[2*oldInd+1];
            }
          }

        }

      }
      */

      free(vertexRemap);
    }
#endif //! USE_INDEXBUFFERCOMPRESSION

    if (out.bad())
      return false;
  }

  return true;
}

#else //USE_LZHAM

//with LZHAM compression
bool
writeBRS(const std::string &filename, const Mesh &mesh)
{
  if (!mesh.isValid())
    return false;

  std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
  if (!out) {
    return false;
  }

  const BRSType type =
    static_cast<BRSType>(VERTICES |
			 (mesh.hasTexCoords() ? TEXCOORDS : VERTICES) |
			 (mesh.hasNormals() ? NORMALS : VERTICES));

  const uint32_t header = HEADER | type;

  out.write((const char *)&header, sizeof(uint32_t));
  if (out.bad())
    return false;

  const uint32_t numVertices = mesh.numVertices();
  out.write((const char *)&numVertices, sizeof(uint32_t));
  if (out.bad())
    return false;
  const uint32_t numTriangles = mesh.numTriangles();
  out.write((const char *)&numTriangles, sizeof(uint32_t));
  if (out.bad())
    return false;

  const size_t sv3 = mesh.numVertices() * 3 * sizeof(float);
  const size_t sv2 = mesh.numVertices() * 2 * sizeof(float);
  size_t totalSize = sv3;
  if (mesh.hasNormals())
    totalSize += sv3;
  if (mesh.hasTexCoords())
    totalSize += sv2;

  unsigned char *BUF = nullptr;

  unsigned char *cBUF = nullptr;

  {
#ifndef USE_INDEXBUFFERCOMPRESSION
    const size_t s = mesh.numTriangles() * 3 * sizeof(float);

    totalSize += s;
    BUF = (unsigned char *)malloc(totalSize);
    if (BUF == nullptr)
      return false;
    cBUF = BUF;

    memcpy(cBUF, mesh.triangles.data(), s);
    cBUF += s;

    {
      memcpy(cBUF, mesh.vertices.data(), sv3);
      cBUF += sv3;
      assert(cBUF <= BUF + totalSize);
    }

    if (mesh.hasTexCoords()) {
      memcpy(cBUF, mesh.texCoords.data(), sv2);
      cBUF += sv2;
      assert(cBUF <= BUF + totalSize);
    }

    if (mesh.hasNormals()) {
      memcpy(cBUF, mesh.normals.data(), sv3);
      cBUF += sv3;
      assert(cBUF <= BUF + totalSize);
    }

#else // USE_INDEXBUFFERCOMPRESSION
    {

#ifndef NDEBUG
      {
        std::cerr << "SAVE mesh.triangles: ";
        //B: was 21 indices whatever the mesh holds
        for (size_t i = 0; i < std::min<size_t>(21, mesh.triangles.size()); ++i)
          std::cerr << mesh.triangles[i] << " ";
        std::cerr << "\n";
      }
#endif //NDEBUG

      const size_t initialBufferCapacity =
        4096; //arbitrary //should be a multiple of 8 and > 0
      WriteBitstream wb(initialBufferCapacity);
      const IndexBufferCompressionFormat format =
        IBCF_PER_TRIANGLE_PREFIX_ENTROPY; //IBCF_PER_TRIANGLE_1; //IBCF_AUTO;
      uint32_t *vertexRemap =
        (uint32_t *)malloc(mesh.numVertices() * sizeof(uint32_t));
      if (vertexRemap == nullptr) {
	return false;
      }
      CompressIndexBuffer(mesh.triangles.data(),
                          mesh.numTriangles(),
                          vertexRemap,
                          mesh.numVertices(),
                          format,
                          wb);
      wb.Finish();

#ifndef NDEBUG
      {
        //print stats
        uint32_t c = 0;
        uint32_t r = 0;
        for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
          c += (vertexRemap[i] == 0xFFFFFFFF);
          r += (vertexRemap[i] != i);
        }
        std::cerr << "number of unused vertices=" << c << "/"
                  << mesh.numVertices() << "\n";
        std::cerr << "number of remapped vertices=" << r << "/"
                  << mesh.numVertices() << "\n";
        int remap = 0;
        for (uint32_t i = 0; i < mesh.numVertices() && remap < 12; ++i) {
          if (i != vertexRemap[i]) {
            std::cerr << i << " -> " << vertexRemap[i] << "\n";
            ++remap;
          }
        }
      }
#endif

      const size_t size = wb.ByteSize();
      const size_t s1 = sizeof(size);

      totalSize += size + s1;

#ifndef NDEBUG
      std::cerr << "totalSize=" << totalSize << "\n";
#endif //NDEBUG

      BUF = static_cast<unsigned char *>( malloc(totalSize) );
      if (BUF == nullptr) {
	return false;
      }
      cBUF = BUF;

      memcpy(cBUF, &size, s1);
      cBUF += s1;
      memcpy(cBUF, wb.RawData(), size);
      cBUF += size;
      assert(cBUF <= BUF + totalSize);

      //copy vertices (with remap)
      {
        float *dst = (float *)cBUF;

        for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < mesh.numVertices());
            assert(oldInd < mesh.numVertices());
            dst[3 * newInd + 0] = mesh.vertices[3 * oldInd + 0];
            dst[3 * newInd + 1] = mesh.vertices[3 * oldInd + 1];
            dst[3 * newInd + 2] = mesh.vertices[3 * oldInd + 2];
          }
        }
        cBUF += sv3;
        assert(cBUF <= BUF + totalSize);
      }

      //copy texcoords if any (with remap)
      {
        if (mesh.hasTexCoords()) {
          float *dst = (float *)cBUF;

          for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices());
              assert(oldInd < mesh.numVertices());
              dst[2 * newInd + 0] = mesh.texCoords[2 * oldInd + 0];
              dst[2 * newInd + 1] = mesh.texCoords[2 * oldInd + 1];
            }
          }
          cBUF += sv2;
          assert(cBUF <= BUF + totalSize);
        }
      }

      //copy normals if any (with remap)
      {
        if (mesh.hasNormals()) {
          float *dst = (float *)cBUF;

          for (uint32_t i = 0; i < mesh.numVertices(); ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices());
              assert(oldInd < mesh.numVertices());
              dst[3 * newInd + 0] = mesh.normals[3 * oldInd + 0];
              dst[3 * newInd + 1] = mesh.normals[3 * oldInd + 1];
              dst[3 * newInd + 2] = mesh.normals[3 * oldInd + 2];
            }
          }

          cBUF += sv3;
          assert(cBUF <= BUF + totalSize);
        }
      }

      /*
      //remap
      {
        const size_t nv3 = mesh.numVertices()*3;
        std::vector<float> v(nv3);

        for (size_t i=0; i<nv3; ++i) {
          v[i] = mesh.vertices[i];
        }

        for (uint32_t i=0; i<mesh.numVertices(); ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < mesh.numVertices());
            assert(oldInd < v.size()/3);
            mesh.vertices[3*newInd+0] = v[3*oldInd+0];
            mesh.vertices[3*newInd+1] = v[3*oldInd+1];
            mesh.vertices[3*newInd+2] = v[3*oldInd+2];
          }
        }

        if (mesh.hasNormals()) {
          for (size_t i=0; i<nv3; ++i) {
            assert(i < v.size());
            v[i] = mesh.normals[i];
          }

          for (uint32_t i=0; i<mesh.numVertices(); ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices());
              assert(oldInd < v.size()/3);
              mesh.normals[3*newInd+0] = v[3*oldInd+0];
              mesh.normals[3*newInd+1] = v[3*oldInd+1];
              mesh.normals[3*newInd+2] = v[3*oldInd+2];
            }
          }

        }

        if (mesh.hasTexCoords()) {
          const size_t nv2 = mesh.numVertices()*2;
          v.resize(nv2);
          for (size_t i=0; i<nv2; ++i) {
            assert(i < v.size());
            v[i] = mesh.normals[i];
          }

          for (uint32_t i=0; i<mesh.numVertices(); ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices());
              assert(oldInd < v.size()/2);
              mesh.texCoords[2*newInd+0] = v[2*oldInd+0];
              mesh.texCoords[2*newInd+1] = v[2*oldInd+1];
            }
          }

        }

      }

      */

      free(vertexRemap);
    }
#endif //USE_INDEXBUFFERCOMPRESSION
  }

  assert(cBUF == BUF + totalSize);

#ifdef USE_LZHAM_HIGHLEVEL

  //use LZHAM higher level function
  // => we get less compression than with lower level functions

  //Warning: do not handle case where data must be split into chunks to be able to be compressed !!!

  unsigned char *BUFCOMP = nullptr;
  size_t totalSizeComp = 0;

  totalSizeComp = lzham_z_compressBound(totalSize);
  BUFCOMP = (unsigned char *)malloc(totalSizeComp);
  if (BUFCOMP == nullptr) {
    return false;
  }

  const int level = Z_BEST_COMPRESSION;

  uLong cmp_len = totalSizeComp;
  const int cmp_status = compress2(BUFCOMP, &cmp_len, BUF, totalSize, level);
  free(BUF);
  if (cmp_status != Z_OK) {
    free(BUFCOMP);
    return false;
  }

  out.write((const char *)&totalSize, sizeof(totalSize));
  out.write((const char *)&cmp_len, sizeof(cmp_len));
  const lzham_uint32 dict_size_log2 = LZHAMTEST_DEFAULT_DICT_SIZE;
  out.write((const char *)&(dict_size_log2), sizeof(dict_size_log2));
  const lzham_uint32 table_update_rate = MY_LZHAM_TABLE_UPDATE_RATE;
  out.write((const char *)&(table_update_rate), sizeof(table_update_rate));
  out.write((const char *)BUFCOMP, cmp_len);

  free(BUFCOMP);

#ifndef NDEBUG
  std::cerr << "original size  =" << totalSize << "\n";
  std::cerr << "compressed size=" << cmp_len << "\n";
  std::cerr << "totalSizeComp=" << totalSizeComp << "\n";
#endif //NDEBUG

#else //USE_LZHAM_HIGHLEVEL

  //Use low level functions of LZHAM
  // (it seems we are able to write more compressed data, see writeBRS()).

  out.write((const char *)&totalSize, sizeof(totalSize));

  const bool writeOk = writeLZHAM(out, (const char *)BUF, totalSize);
  free(BUF);
  if (!writeOk) {
    return false;
  }

#endif //USE_LZHAM_HIGHLEVEL

  if (out.bad())
    return false;

  return true;
}

#endif //USE_LZHAM

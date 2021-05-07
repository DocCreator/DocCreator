#include "brs.hpp"

#include <cassert>
#include <cstring> //memcpy
#include <fstream>
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
  if (in.bad()) {
    return false;
  }

  if ((t & HEADER_MASK) != HEADER) {
    return false;
  }
  type = static_cast<BRSType>(t & TYPE_MASK);

  if (type != VERTICES
      && type != (VERTICES | TEXCOORDS)
      && type != (VERTICES | NORMALS)
      && type != (VERTICES | TEXCOORDS | NORMALS)) {
    return false;
  }

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
  Read buffer @a data of size @a szb from input stream @a in compressed with LZHAM.

  @param[in] szb  size in Bytes to read

 */
bool
readLZHAM(std::istream &in, char *data, size_t szb)
{

  lzham_uint32 dict_size_log2 = 0;
  in.read((char *)&dict_size_log2, sizeof(dict_size_log2));
  if (in.bad())
    return false;

  lzham_uint32 table_update_rate = 0;
  in.read((char *)&table_update_rate, sizeof(table_update_rate));
  if (in.bad())
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

    size_t comp_size;
    in.read((char *)&comp_size, sizeof(comp_size));
    //std::cerr<<"readC "<<comp_size<<"\n";

    if (srcSize < comp_size) {
      free(src);
      src = nullptr;
      srcSize = comp_size;
      src = malloc(srcSize);
      if (src == nullptr)
	return false;
    }

    in.read((char *)src, comp_size);
    if (in.bad()) {
      std::cerr << "LZHAM: failed to read compressed data\n";
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

    shift += uncomp_size;
    assert(uncomp_size <= toReadSize);
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
    const size_t s = mesh.numTriangles * 3 * sizeof(uint32_t);
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
                << " mesh.numTriangles*3*sizeof(uint32_t)="
                << mesh.numTriangles * 3 * sizeof(uint32_t) << "\n";
#endif
      ReadBitstream rb(data, size8);
      DecompressIndexBuffer(mesh.triangles, mesh.numTriangles, rb);
      free(data);

#ifndef NDEBUG
      {
        std::cerr << "READ mesh.triangles: ";
        for (size_t i = 0; i < 21; ++i)
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
  assert(mesh.numTriangles == numTris);

  assert(type & VERTICES);
  {
    mesh.allocateVertices(numVertices);

    const size_t s = mesh.numVertices * 3 * sizeof(float);

    in.read((char *)mesh.vertices, s);
    if (in.bad()) {
      mesh.clear();
      return false;
    }
  }
  assert(mesh.numVertices == numVertices);

  if (type & TEXCOORDS) {
    mesh.allocateTexCoords();

    const size_t s = mesh.numVertices * 2 * sizeof(float);

    in.read((char *)mesh.texCoords, s);
    if (in.bad()) {
      mesh.clear();
      return false;
    }

    assert(mesh.hasTexCoords());
  }

  if (type & NORMALS) {
    mesh.allocateNormals();

    const size_t s = mesh.numVertices * 3 * sizeof(float);

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

  size_t totalSize = 0;
  in.read((char *)&totalSize, sizeof(totalSize));
  if (in.bad())
    return false;

#ifdef USE_LZHAM_HIGHLEVEL

  //Warning: we do not check buffer size ! We consider that we only have one chunk of data

  uLong cmp_len = 0;
  in.read((char *)&cmp_len, sizeof(cmp_len));
  if (in.bad())
    return false;

  lzham_uint32 dict_size_log2 = 0;
  in.read((char *)&dict_size_log2, sizeof(dict_size_log2));
  if (in.bad())
    return false;

  lzham_uint32 table_update_rate = 0;
  in.read((char *)&table_update_rate, sizeof(table_update_rate));
  if (in.bad())
    return false;

#ifndef NDEBUG
  std::cerr << "totalSize=" << totalSize << "\n";
  std::cerr << "cmp_len=" << cmp_len << "\n";
  std::cerr << "dict_size_log2=" << dict_size_log2 << "\n";
  std::cerr << "table_update_rate=" << table_update_rate << "\n";
#endif //NDEBUG

  unsigned char *BUFCOMP = (unsigned char *)malloc(cmp_len);
  if (BUFCOMP == nullptr)
    return false;

  in.read((char *)BUFCOMP, cmp_len);
  if (in.bad()) {
    free(BUFCOMP);
    return false;
  }

  const size_t totalSizeComp = cmp_len;

  unsigned char *BUF = (unsigned char *)malloc(totalSize);
  if (BUF == nullptr) {
    free(BUFCOMP);
    return false;
  }

  uLong uncomp_len = totalSize;
  const int cmp_status = uncompress(BUF, &uncomp_len, BUFCOMP, cmp_len);
  if (cmp_status != Z_OK || totalSize != uncomp_len) {
    free(BUF);
    free(BUFCOMP);
    return false;
  }

#else
  //Use low level functions of LZHAM
  // (it seems we are able to write more compressed data, see writeBRS()).

  unsigned char *BUF = static_cast<unsigned char *>( malloc(totalSize) );
  if (BUF == nullptr) {
    return false;
  }

  const bool readOk = readLZHAM(in, (char *)BUF, totalSize);
  if (!readOk) {
    return false;
  }

  size_t totalSizeComp = 0;
  unsigned char *BUFCOMP = nullptr;

#endif //USE_LZHAM_HIGHLEVEL

  unsigned char *cBUF = BUF;

#ifndef USE_INDEXBUFFERCOMPRESSION
  {
    free(BUFCOMP);

    mesh.allocateTriangles(numTris);

    const size_t st3 = numTris * 3 * sizeof(int32_t);
    memcpy(mesh.triangles, cBUF, st3);
    cBUF += st3;
  }
#else
  {
    mesh.allocateTriangles(numTris);

    size_t size = 0;
    size_t s1 = sizeof(size);
    memcpy(&size, cBUF, s1);
    cBUF += s1;

    size_t size8 = (size + 7) & (~7); //must be a multiple of 8 bytes.

    //use BUFCOMP as temporary buffer
    if (size8 > totalSizeComp) {
      free(BUFCOMP);
      BUFCOMP = static_cast<unsigned char *>( malloc(size8) );
      if (BUFCOMP == nullptr)
	return false;
    }

    assert(BUFCOMP);
    memcpy(BUFCOMP, cBUF, size);
    cBUF += size;

    ReadBitstream rb(BUFCOMP, size8);
    DecompressIndexBuffer(mesh.triangles, mesh.numTriangles, rb);

    free(BUFCOMP);

#ifndef NDEBUG
    {
      std::cerr << "READ mesh.triangles: ";
      for (size_t i = 0; i < 21; ++i)
        std::cerr << mesh.triangles[i] << " ";
      std::cerr << "\n";
    }
#endif //NDEBUG
  }
#endif //USE_INDEXBUFFERCOMPRESSION
  assert(mesh.numTriangles == numTris);

  const size_t sv3 = numVertices * 3 * sizeof(float);
  const size_t sv2 = numVertices * 2 * sizeof(float);

  assert(type & VERTICES);
  {
    mesh.allocateVertices(numVertices);

    memcpy(mesh.vertices, cBUF, sv3);
    cBUF += sv3;
  }
  assert(mesh.numVertices == numVertices);

  if ((type & TEXCOORDS) != 0) {
    mesh.allocateTexCoords();

    memcpy(mesh.texCoords, cBUF, sv2);
    cBUF += sv2;

    assert(mesh.hasTexCoords());
  }

  if ((type & NORMALS) != 0) {
    mesh.allocateNormals();

    memcpy(mesh.normals, cBUF, sv3);
    cBUF += sv3;

    assert(mesh.hasNormals());
  }

  assert((size_t)(cBUF - BUF) == totalSize);

  assert(mesh.isValid());

  return true;
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

  out.write((const char *)&(mesh.numVertices), sizeof(uint32_t));
  if (out.bad())
    return false;
  out.write((const char *)&(mesh.numTriangles), sizeof(uint32_t));
  if (out.bad())
    return false;

  {
#ifndef USE_INDEXBUFFERCOMPRESSION
    const size_t s = mesh.numTriangles * 3 * sizeof(float);
    out.write((const char *)mesh.triangles, s);

    {
      const size_t s = mesh.numVertices * 3 * sizeof(float);
      out.write((const char *)mesh.vertices, s);
      if (out.bad())
        return false;
    }

    if (mesh.hasTexCoords()) {
      const size_t s = mesh.numVertices * 2 * sizeof(float);
      out.write((const char *)mesh.texCoords, s);
      if (out.bad())
        return false;
    }

    if (mesh.hasNormals()) {
      const size_t s = mesh.numVertices * 3 * sizeof(float);
      out.write((const char *)mesh.normals, s);
      if (out.bad())
        return false;
    }

#else
    {
#ifndef NDEBUG
      {
        std::cerr << "SAVE mesh.triangles: ";
        for (size_t i = 0; i < 21; ++i)
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
        (uint32_t *)malloc(mesh.numVertices * sizeof(uint32_t));
      if (vertexRemap == nullptr) {
	return false;
      }
      CompressIndexBuffer(mesh.triangles,
                          mesh.numTriangles,
                          vertexRemap,
                          mesh.numVertices,
                          format,
                          wb);
      wb.Finish();

#ifndef NDEBUG
      {
        uint32_t c = 0;
        uint32_t r = 0;
        for (uint32_t i = 0; i < mesh.numVertices; ++i) {
          c += (vertexRemap[i] == 0xFFFFFFFF);
          r += (vertexRemap[i] != i);
        }
        std::cerr << "number of unused vertices=" << c << "/"
                  << mesh.numVertices << "\n";
        std::cerr << "number of remapped vertices=" << r << "/"
                  << mesh.numVertices << "\n";
        int remap = 0;
        for (uint32_t i = 0; i < mesh.numVertices && remap < 12; ++i) {
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
      std::cerr << "original size  =" << mesh.numTriangles * 3 * sizeof(float)
                << "\n";
      std::cerr << "compressed size=" << size << "\n";
#endif //NDEBUG

      //save vertices (with remap)
      {
        const size_t nv3 = mesh.numVertices * 3;
        std::vector<float> v(nv3);

        for (uint32_t i = 0; i < mesh.numVertices; ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < v.size() / 3);
            assert(oldInd < mesh.numVertices);
            v[3 * newInd + 0] = mesh.vertices[3 * oldInd + 0];
            v[3 * newInd + 1] = mesh.vertices[3 * oldInd + 1];
            v[3 * newInd + 2] = mesh.vertices[3 * oldInd + 2];
          }
        }
        const size_t s = nv3 * sizeof(float);
        out.write((const char *)&v[0], s);
        if (out.bad())
          return false;
      }

      //save texcoords if any (with remap)
      if (mesh.hasTexCoords()) {
        const size_t nv2 = mesh.numVertices * 2;
        std::vector<float> v(nv2);

        for (uint32_t i = 0; i < mesh.numVertices; ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < v.size() / 2);
            assert(oldInd < mesh.numVertices);
            v[2 * newInd + 0] = mesh.texCoords[2 * oldInd + 0];
            v[2 * newInd + 1] = mesh.texCoords[2 * oldInd + 1];
          }
        }
        const size_t s = nv2 * sizeof(float);
        out.write((const char *)&v[0], s);
        if (out.bad())
          return false;
      }

      //save normals if any (with remap)
      if (mesh.hasNormals()) {
        const size_t nv3 = mesh.numVertices * 3;
        std::vector<float> v(nv3);

        for (uint32_t i = 0; i < mesh.numVertices; ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < v.size() / 3);
            assert(oldInd < mesh.numVertices);
            v[3 * newInd + 0] = mesh.normals[3 * oldInd + 0];
            v[3 * newInd + 1] = mesh.normals[3 * oldInd + 1];
            v[3 * newInd + 2] = mesh.normals[3 * oldInd + 2];
          }
        }
        const size_t s = nv3 * sizeof(float);
        out.write((const char *)&v[0], s);
        if (out.bad())
          return false;
      }

      /*
      //remap
      {
        const size_t nv3 = mesh.numVertices*3;
        std::vector<float> v(nv3);

        for (size_t i=0; i<nv3; ++i) {
          v[i] = mesh.vertices[i];
        }

        for (uint32_t i=0; i<mesh.numVertices; ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < mesh.numVertices);
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

          for (uint32_t i=0; i<mesh.numVertices; ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices);
              assert(oldInd < v.size()/3);
              mesh.normals[3*newInd+0] = v[3*oldInd+0];
              mesh.normals[3*newInd+1] = v[3*oldInd+1];
              mesh.normals[3*newInd+2] = v[3*oldInd+2];
            }
          }

        }

        if (mesh.hasTexCoords()) {
          const size_t nv2 = mesh.numVertices*2;
          v.resize(nv2);
          for (size_t i=0; i<nv2; ++i) {
            assert(i < v.size());
            v[i] = mesh.normals[i];
          }

          for (uint32_t i=0; i<mesh.numVertices; ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices);
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

  out.write((const char *)&(mesh.numVertices), sizeof(uint32_t));
  if (out.bad())
    return false;
  out.write((const char *)&(mesh.numTriangles), sizeof(uint32_t));
  if (out.bad())
    return false;

  const size_t sv3 = mesh.numVertices * 3 * sizeof(float);
  const size_t sv2 = mesh.numVertices * 2 * sizeof(float);
  size_t totalSize = sv3;
  if (mesh.hasNormals())
    totalSize += sv3;
  if (mesh.hasTexCoords())
    totalSize += sv2;

  unsigned char *BUF = nullptr;

  unsigned char *cBUF = nullptr;

  {
#ifndef USE_INDEXBUFFERCOMPRESSION
    const size_t s = mesh.numTriangles * 3 * sizeof(float);

    totalSize += s;
    BUF = (unsigned char *)malloc(totalSize);
    if (BUF == nullptr)
      return false;
    cBUF = BUF;

    memcpy(cBUF, mesh.triangles, s);
    cBUF += s;

    {
      memcpy(cBUF, mesh.vertices, sv3);
      cBUF += sv3;
      assert(cBUF <= BUF + totalSize);
    }

    if (mesh.hasTexCoords()) {
      memcpy(cBUF, mesh.texCoords, sv2);
      cBUF += sv2;
      assert(cBUF <= BUF + totalSize);
    }

    if (mesh.hasNormals()) {
      memcpy(cBUF, mesh.normals, sv3);
      cBUF += sv3;
      assert(cBUF <= BUF + totalSize);
    }

#else // USE_INDEXBUFFERCOMPRESSION
    {

#ifndef NDEBUG
      {
        std::cerr << "SAVE mesh.triangles: ";
        for (size_t i = 0; i < 21; ++i)
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
        (uint32_t *)malloc(mesh.numVertices * sizeof(uint32_t));
      if (vertexRemap == nullptr) {
	return false;
      }
      CompressIndexBuffer(mesh.triangles,
                          mesh.numTriangles,
                          vertexRemap,
                          mesh.numVertices,
                          format,
                          wb);
      wb.Finish();

#ifndef NDEBUG
      {
        //print stats
        uint32_t c = 0;
        uint32_t r = 0;
        for (uint32_t i = 0; i < mesh.numVertices; ++i) {
          c += (vertexRemap[i] == 0xFFFFFFFF);
          r += (vertexRemap[i] != i);
        }
        std::cerr << "number of unused vertices=" << c << "/"
                  << mesh.numVertices << "\n";
        std::cerr << "number of remapped vertices=" << r << "/"
                  << mesh.numVertices << "\n";
        int remap = 0;
        for (uint32_t i = 0; i < mesh.numVertices && remap < 12; ++i) {
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

        for (uint32_t i = 0; i < mesh.numVertices; ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < mesh.numVertices);
            assert(oldInd < mesh.numVertices);
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

          for (uint32_t i = 0; i < mesh.numVertices; ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices);
              assert(oldInd < mesh.numVertices);
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

          for (uint32_t i = 0; i < mesh.numVertices; ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices);
              assert(oldInd < mesh.numVertices);
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
        const size_t nv3 = mesh.numVertices*3;
        std::vector<float> v(nv3);

        for (size_t i=0; i<nv3; ++i) {
          v[i] = mesh.vertices[i];
        }

        for (uint32_t i=0; i<mesh.numVertices; ++i) {
          const uint32_t oldInd = i;
          const uint32_t newInd = vertexRemap[i];
          if (newInd != 0xFFFFFFFF) {
            assert(newInd < mesh.numVertices);
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

          for (uint32_t i=0; i<mesh.numVertices; ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices);
              assert(oldInd < v.size()/3);
              mesh.normals[3*newInd+0] = v[3*oldInd+0];
              mesh.normals[3*newInd+1] = v[3*oldInd+1];
              mesh.normals[3*newInd+2] = v[3*oldInd+2];
            }
          }

        }

        if (mesh.hasTexCoords()) {
          const size_t nv2 = mesh.numVertices*2;
          v.resize(nv2);
          for (size_t i=0; i<nv2; ++i) {
            assert(i < v.size());
            v[i] = mesh.normals[i];
          }

          for (uint32_t i=0; i<mesh.numVertices; ++i) {
            const uint32_t oldInd = i;
            const uint32_t newInd = vertexRemap[i];
            if (newInd != 0xFFFFFFFF) {
              assert(newInd < mesh.numVertices);
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

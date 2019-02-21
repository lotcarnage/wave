#ifndef SAG_INCLUDED_SAG_FORMAT_WAVE_HPP
#define SAG_INCLUDED_SAG_FORMAT_WAVE_HPP

#include <cstdint>
#include <cstddef>

namespace sag { namespace audio { namespace format {

/* version numbers */
constexpr std::uint32_t major_version = 0;
constexpr std::uint32_t minor_version = 1;
constexpr std::uint32_t patch_version = 0;

class Wave {
public:
  enum class SampleFormat;
private:
  struct RiffHeader {
    char four_cc[4];
    std::uint32_t num_body_bytes;
  };

  struct FmtChunkHeader {
    char four_cc[4];
    std::uint32_t num_chunk_bytes;
    std::uint16_t format_id;
    std::uint16_t num_channels;
    std::uint32_t sampling_rate;
    std::uint32_t byte_par_sec;
    std::uint16_t num_block_bytes;
    std::uint16_t bit_par_samples;
  };

  struct DataChunkHeader {
    char four_cc[4];
    std::uint32_t num_data_bytes;
  };

  struct WaveChunkHeader {
    char four_cc[4];
    FmtChunkHeader fmt_chunk_header;
    DataChunkHeader data_chunk_header;
  };

  static constexpr std::size_t minimum_wave_bytes =
    sizeof(RiffHeader) + sizeof(FmtChunkHeader) + sizeof(DataChunkHeader);

  static inline bool CompareFourCC(
    char const (&bytes)[4], char const (&fourcc)[5])
  {
    return
      (bytes[0] == fourcc[0]) &&
      (bytes[1] == fourcc[1]) &&
      (bytes[2] == fourcc[2]) &&
      (bytes[3] == fourcc[3]);
  }

  static inline SampleFormat
  GetSampleFormat(std::uint16_t format_id, std::uint16_t bit_par_samples)
  {
    switch (format_id) {
    case 0x0001: /* PCM */
      switch (bit_par_samples) {
      case 16:
        return SampleFormat::Int16;
      case 32:
        return SampleFormat::Int32;
      default:
        return SampleFormat::Invalid;
      }
    case 0x0003: /* IEEE float 32bit */
      if (bit_par_samples != 32) {
        return SampleFormat::Invalid;
      }
      return SampleFormat::Float32;
    }
    return SampleFormat::Invalid;
  }

public:
  enum class SampleFormat {
    Invalid,
    Int16,
    Int32,
    Float32
  };

  struct Info {
    bool is_valid;
    std::uint32_t num_channels;
    std::uint32_t num_samples;
    std::uint32_t sampling_rate;
    SampleFormat sample_format;
    void* data;
  };

  static inline Info
  ReadWaveInfo(std::uint8_t const *bytes, std::size_t num_bytes)
  {
    if (num_bytes < minimum_wave_bytes) {
      return {false, 0, };
    }

    const RiffHeader* riff_header =
      reinterpret_cast<const RiffHeader*>(bytes);
    const WaveChunkHeader* wave_chunk_header =
      reinterpret_cast<const WaveChunkHeader*>(bytes + sizeof(RiffHeader));
    const FmtChunkHeader* fmt_chunk_header =
      &wave_chunk_header->fmt_chunk_header;
    const DataChunkHeader* data_chunk_header =
      &wave_chunk_header->data_chunk_header;
    if (!CompareFourCC(riff_header->four_cc, "RIFF") ||
        !CompareFourCC(wave_chunk_header->four_cc, "WAVE") ||
        !CompareFourCC(fmt_chunk_header->four_cc, "fmt ") ||
        !CompareFourCC(data_chunk_header->four_cc, "data")) {
      return {false, 0, };
    }

    SampleFormat sample_format =
      GetSampleFormat(fmt_chunk_header->format_id,
		      fmt_chunk_header->bit_par_samples);
    if (sample_format == SampleFormat::Invalid) {
      return {false, 0, };
    }

    std::uint32_t num_samples =
      data_chunk_header->num_data_bytes / fmt_chunk_header->num_block_bytes;

    std::uint8_t const* data =
      bytes + sizeof(RiffHeader) + sizeof(WaveChunkHeader);

    return {
      true,
      fmt_chunk_header->num_channels,
      num_samples,
      fmt_chunk_header->sampling_rate,
      sample_format,
      const_cast<void*>(static_cast<void const*>(data))
    };
  }
}; /* class Wave */

}}}

#endif /* SAG_INCLUDED_SAG_FORMAT_WAVE_HPP */

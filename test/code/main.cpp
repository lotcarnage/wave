/* test target module API */
#include <sag/audio/format/wave.hpp>

#include <cstdint>
#include <cstddef>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

const char test_data_path[] = "../data/voice_a.wav";

std::int64_t GetFileSize(FILE* fp)
{
  struct stat stat_obj;
  int fd = fileno(fp);
  if (fd == -1) {
    return -1;
  }
  if (fstat(fd, &stat_obj) < 0) {
    return -1;
  }
  return stat_obj.st_size;
}

int main(int argc, char* argv[])
{
  FILE* fp = fopen(test_data_path, "rb");
  if (fp == nullptr) {
    return EXIT_FAILURE;
  }
  std::int64_t file_size = GetFileSize(fp);
  if (file_size <= 0) {
    return EXIT_FAILURE;
  }

  std::size_t wave_file_size =
    static_cast<std::size_t>(file_size);
  std::uint8_t* wave_file_data =
    static_cast<std::uint8_t*>(malloc(wave_file_size));
  fread(wave_file_data, wave_file_size, 1, fp);
  fclose(fp);

  sag::audio::format::Wave::Info wave_info =
    sag::audio::format::Wave::ReadWaveInfo(wave_file_data, wave_file_size);

  int result = EXIT_SUCCESS;
  if (!wave_info.is_valid) {
    result = EXIT_FAILURE;
  }

  if (wave_info.num_channels != 1) {
    result = EXIT_FAILURE;
  }
  if (wave_info.num_samples != 188262) {
    result = EXIT_FAILURE;
  }
  if (wave_info.sampling_rate != 44100) {
    result = EXIT_FAILURE;
  }
  if (wave_info.sample_format !=
      sag::audio::format::Wave::SampleFormat::Int16) {
    result = EXIT_FAILURE;
  }
  if (wave_info.data == nullptr) {
    result = EXIT_FAILURE;
  }

  free(wave_file_data);
  return result;
}

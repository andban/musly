/**
 * Copyright 2026, Andreas
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "wav_writer.h"
#include <fstream>
#include <cstdint>
#include <algorithm>
#include "minilog.h"

namespace musly {
namespace decoders {

namespace {

struct WavHeader {
    char riff_header[4];        // Contains "RIFF"
    uint32_t wav_size;          // Size of the entire file in bytes minus 8 bytes
    char wave_header[4];        // Contains "WAVE"
    char fmt_header[4];         // Contains "fmt "
    uint32_t fmt_chunk_size;    // 16 for PCM
    uint16_t audio_format;      // 1 for PCM, 3 for IEEE Float
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;         // sample_rate * num_channels * bytes_per_sample
    uint16_t sample_alignment;  // num_channels * bytes_per_sample
    uint16_t bit_depth;
    char data_header[4];        // Contains "data"
    uint32_t data_bytes;        // Number of bytes in data
};

} // namespace

bool wav_write_22050hz_mono_float(const std::string& filename, const std::vector<float>& samples) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        MINILOG(logERROR) << "wav_writer: Could not open file for writing: " << filename;
        return false;
    }

    uint32_t num_samples = static_cast<uint32_t>(samples.size());
    uint16_t num_channels = 1;
    uint32_t sample_rate = 22050;
    uint16_t bit_depth = 32; // IEEE Float 32-bit
    uint16_t bytes_per_sample = bit_depth / 8;

    WavHeader header;
    std::copy_n("RIFF", 4, header.riff_header);
    header.wav_size = sizeof(WavHeader) - 8 + num_samples * bytes_per_sample;
    std::copy_n("WAVE", 4, header.wave_header);
    std::copy_n("fmt ", 4, header.fmt_header);
    header.fmt_chunk_size = 16;
    header.audio_format = 3; // IEEE Float
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * num_channels * bytes_per_sample;
    header.sample_alignment = num_channels * bytes_per_sample;
    header.bit_depth = bit_depth;
    std::copy_n("data", 4, header.data_header);
    header.data_bytes = num_samples * bytes_per_sample;

    file.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
    file.write(reinterpret_cast<const char*>(samples.data()), num_samples * bytes_per_sample);

    if (file.fail()) {
        MINILOG(logERROR) << "wav_writer: Error writing to file: " << filename;
        return false;
    }

    file.close();
    return true;
}

} // namespace decoders
} // namespace musly

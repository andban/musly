/**
 * Copyright 2026, Andreas
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MUSLY_DECODERS_WAV_WRITER_H_
#define MUSLY_DECODERS_WAV_WRITER_H_

#include <string>
#include <vector>

namespace musly {
namespace decoders {

/**
 * Writes a vector of floating point mono samples with a sample rate of 22050Hz
 * to a .wav file.
 *
 * @param filename The path to the .wav file to be written.
 * @param samples  A vector of floating point mono samples.
 * @return true if successful, false otherwise.
 */
bool wav_write_22050hz_mono_float(const std::string& filename, const std::vector<float>& samples);

} // namespace decoders
} // namespace musly

#endif // MUSLY_DECODERS_WAV_WRITER_H_

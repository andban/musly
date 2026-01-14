/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "decoder.h"
#include "plugins.h"

namespace
{
/**
 * Adjust the requested excerpt start position and length to stay within the boundary of the total duration
 * of the input audio data.
 *
 * When excerpt length is <=0, take the whole duration of the audio
 * When excerpt start is <0, it uses it as an offset to half of the duration
 * @param total_duration duration in seconds of the audio we want to take an excerpt.
 * @param excerpt_start requested start position in seconds of the excerpt.
 * @param excerpt_length requested length of the excerpt.
 */
void adjustExcerptBounds(const float total_duration, float& excerpt_start, float& excerpt_length)
{
    excerpt_length = std::min(excerpt_length, total_duration);
    if (excerpt_length <= 0)
    {
        excerpt_start = 0;
        excerpt_length = total_duration;
    }
    else if (excerpt_start < 0)
    {
        excerpt_start = std::min(-excerpt_start, (total_duration - excerpt_length) / 2);
    }
    else if (excerpt_length + excerpt_start > total_duration)
    {
        excerpt_start = std::max(0.0f, total_duration - excerpt_length);
        excerpt_length = std::min(total_duration, total_duration - excerpt_start);
    }
}

}

namespace musly
{

std::vector<float>
decoder::decodeto_22050hz_mono_float(
        const std::string& filename,
        float excerpt_length,
        float excerpt_start)
{
    const std::unique_ptr<decoder_file> file = open_file(filename);
    if (!file || !file->open())
    {
        MINILOG(logERROR) << "Could not open file for decoding: " << filename;
        return std::vector<float>(0);
    }

    adjustExcerptBounds(file->duration(), excerpt_start, excerpt_length);
    MINILOG(logDEBUG) << "decoder: Will decode from " << excerpt_start << " to " << (excerpt_start + excerpt_length);

    file->seek(excerpt_start);

    const auto samples_needed = static_cast<size_t>(excerpt_length * 22050);
    std::vector<float> decoded_pcm(samples_needed);

    const auto samples_read = file->read(samples_needed, &decoded_pcm[0]);
    MINILOG(logTRACE) << "decoder: " << filename << " decoding finalized.";

    return {decoded_pcm.begin(), decoded_pcm.begin() + samples_read};
}


} /* namespace musly */

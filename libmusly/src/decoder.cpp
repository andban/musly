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

#include <decoders/wav_writer.h>

#include "plugins.h"

namespace musly {

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

    excerpt_length = std::min(excerpt_length, file->duration());
    if (excerpt_length <= 0)
    {
        excerpt_start = 0;
        excerpt_length = file->duration();
    }
    else if (excerpt_start < 0)
    {
        excerpt_start = std::min(-excerpt_start, (file->duration() - excerpt_length) / 2);
    }
    else if (excerpt_length + excerpt_start > file->duration())
    {
        excerpt_start = std::max(0.0f, file->duration() - excerpt_length);
        excerpt_length = std::min(file->duration(), file->duration() - excerpt_start);
    }

    MINILOG(logDEBUG) << "decoder: Will decode from " << excerpt_start << " to " << (excerpt_start + excerpt_length);

    file->seek(excerpt_start);
    size_t samples_needed = excerpt_length * 22050;
    std::vector<float> decoded_pcm(samples_needed);
    size_t samples_read = file->read(samples_needed, &decoded_pcm[0]);

    MINILOG(logTRACE) << "decoder: " << filename << " decoding finalized.";

    return {&decoded_pcm[0], &decoded_pcm[samples_read]};
}


} /* namespace musly */

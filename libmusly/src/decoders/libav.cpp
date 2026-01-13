/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *           2014-2016, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <vector>
#include <algorithm>

#include "libav_file.h"
#include "libav_functions.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "minilog.h"
#include "resampler.h"
#include "libav.h"


namespace musly::decoders
{
MUSLY_DECODER_REGIMPL_DYNAMIC(libav, 0, libav_is_available);

libav::libav()
{
}

void
libav_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
{
    if (level <= av_log_get_level())
    {
        vfprintf(FileLogger::get_stream(), fmt, vargs);
    }
}

std::vector<float>
libav::decodeto_22050hz_mono_float(
    const std::string& file,
    float excerpt_length,
    float excerpt_start)
{
    MINILOG(logTRACE) << "Decoding: " << file << " started.";

    // show libav messages only in verbose mode
    if (MiniLog::current_level() >= logTRACE)
    {
        av_log_set_level(AV_LOG_VERBOSE);
        av_log_set_callback(libav_log_callback);
    }
    else
    {
        av_log_set_level(AV_LOG_PANIC);
    }

    libav_file audio_file(file);
    if (!audio_file.open())
    {
        MINILOG(logERROR) << "Could not open file: " << file;
        return std::vector<float>(0);
    }

    MINILOG(logDEBUG) << "libav: Audio file duration : " << audio_file.duration() << " seconds";

    if (excerpt_length <= 0 || excerpt_length > audio_file.duration())
    {
        excerpt_start = 0;
        excerpt_length = audio_file.duration();
    }
    else if (excerpt_start < 0)
    {
        excerpt_start = std::min(-excerpt_start, (audio_file.duration() - excerpt_length) / 2);
    }
    else if (excerpt_length + excerpt_start > audio_file.duration())
    {
        excerpt_start = std::max(0.0f, audio_file.duration() - excerpt_length);
        excerpt_length = std::min(audio_file.duration(), audio_file.duration() - excerpt_start);
    }

    MINILOG(logDEBUG) << "libav: Will decode from " << excerpt_start << " to " << (excerpt_start + excerpt_length);
    audio_file.seek(excerpt_start);

    size_t samples_to_read = excerpt_length * audio_file.sample_rate();
    std::vector<float> pcm(samples_to_read);

    size_t samples_read = audio_file.read(samples_to_read, pcm.data());
    if (samples_read < samples_to_read)
    {
        pcm.resize(samples_read);
    }

    MINILOG(logTRACE) << "Decoding: " << file << " finalized.";
    return pcm;
}
} /* namespace musly::decoders */


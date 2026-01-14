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
#include "libav.h"


namespace musly::decoders
{
MUSLY_DECODER_REGIMPL_DYNAMIC(libav, 0, libav_is_available);

libav::libav()
{
    MINILOG(logTRACE) << "libav: Created LibAV decoder.";
}

void
libav_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
{
    if (level <= av_log_get_level())
    {
        vfprintf(FileLogger::get_stream(), fmt, vargs);
    }
}

std::unique_ptr<decoder_file>
libav::open_file(const std::string& filename)
{
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

    return std::make_unique<libav_file>(filename);

}
} /* namespace musly::decoders */


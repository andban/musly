/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "coreaudio.h"
#include "minilog.h"
#include "coreaudio_file.h"

#include <algorithm>
#include <numeric>

#include <AudioToolbox/AudioToolbox.h>
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AudioFile.h>


namespace musly::decoders {

MUSLY_DECODER_REGIMPL(coreaudio, 0);

coreaudio::coreaudio()
{
    MINILOG(logTRACE) << "coreaudio: Created CoreAudio decoder.";
}

std::unique_ptr<decoder_file>
coreaudio::open_file(const std::string& filename)
{
    return std::make_unique<coreaudio_file>(filename);
}

} // namespace musly::decoders

/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "minilog.h"
#include "none.h"

namespace musly { namespace decoders {

MUSLY_DECODER_REGIMPL(none, 0);

none::none()
{
    MINILOG(logTRACE) << "none: Created 'none' decoder.";
}

std::unique_ptr<decoder_file>
none::open_file(const std::string&)
{
    return nullptr;
}

}} // namespace musly::decoders

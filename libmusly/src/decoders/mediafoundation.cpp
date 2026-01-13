/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mediafoundation.h"
#include "mediafoundation_file.h"
#include "minilog.h"

#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <limits>


namespace musly { namespace decoders {


MUSLY_DECODER_REGIMPL(mediafoundation, 0);


mediafoundation::mediafoundation()
{
    MINILOG(logTRACE) << "mediafoundation: Create MediaFoundation decoder.";
}

std::unique_ptr<decoder_file>
mediafoundation::open_file(const std::string& filename)
{
    return std::make_unique<mediafoundation_file>(filename);
}


}} // namespace musly::decoders

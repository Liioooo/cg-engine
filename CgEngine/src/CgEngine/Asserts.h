#pragma once

#include "Logging.h"

#ifdef CG_ENABLE_DEBUG_FEATURES
    #include <iostream>
    #define CG_ASSERT(check, message) { if (!(check)) { CG_LOGGING_ERROR(message) std::cout << std::flush; }}
#else
    #define CG_ASSERT(...)
#endif

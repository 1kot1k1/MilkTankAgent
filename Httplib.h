#pragma once

// Single, consistent inclusion point for httplib.h. Every .cpp file
// that needs the HTTP client or server must include THIS file
// instead of "httplib.h" directly — including it with different
// preprocessor settings (e.g. SSL support defined in one file but
// not another) in different translation units can lead to mismatched
// object layouts and crashes when the program is linked together.

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"
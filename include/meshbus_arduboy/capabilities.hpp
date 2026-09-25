/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_CAPABILITIES_HPP_
#define MESHBUS_ARDUBOY_CAPABILITIES_HPP_
#define MESHBUS_ARDUBOY_SDK_VERSION "0.2.0-dev.1"
#define MESHBUS_ARDUBOY_SEMANTIC_REVISION 1
#ifndef MESHBUS_ARDUBOY_COMPATIBILITY
#define MESHBUS_ARDUBOY_COMPATIBILITY 0
#endif
#if MESHBUS_ARDUBOY_COMPATIBILITY
#pragma message("sdk-arduboy compatibility mode: legacy placeholders remain; see capabilities.json")
#define MESHBUS_ARDUBOY_UNSUPPORTED(feature) ((void)0)
#else
// An external error-marked call survives inlining; annotating a no-op inline
// wrapper alone could let optimization silently remove the diagnostic.
#define MESHBUS_ARDUBOY_UNSUPPORTED(feature) do { \
    extern void meshbus_arduboy_unsupported_##feature() \
        __attribute__((error("sdk-arduboy unsupported " #feature "; select explicit COMPATIBILITY only for a reviewed degradation"))); \
    meshbus_arduboy_unsupported_##feature(); \
} while (0)
#endif
#endif

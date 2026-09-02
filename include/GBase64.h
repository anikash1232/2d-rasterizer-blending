/**
 *  Copyright 2026 Mike Reed
 */

#ifndef GBase64_DEFINED
#define GBase64_DEFINED

#include "GData.h"

std::shared_ptr<GData> GEncodeToBase64(const void* src, size_t size, const char prefix[] = nullptr);
std::shared_ptr<GData> GDecodeFromBase64(const void* src, size_t size);

#endif

/**
 *  Copyright 2025 Mike Reed
 */

#include "../include/GData.h"

static void free_do_nothing(void*) {}

static void free_uint8_array(void* ptr) {
    auto array = static_cast<uint8_t*>(ptr);
    delete[] array;
}

GData::GData(const void* ptr, size_t size, ReleaseProc proc)
    : m_releaseProc(std::move(proc ? proc : free_do_nothing))
    , m_data(const_cast<void*>(ptr))
    , m_size(size)
{}

std::shared_ptr<GData> GData::Empty() {
    static std::shared_ptr<GData> gEmpty;
    if (!gEmpty) {
        gEmpty = std::shared_ptr<GData>(new GData(nullptr, 0, {}));
    }
    return gEmpty;
}

std::shared_ptr<GData> GData::Uninitialized(size_t size) {
    if (size == 0) {
        return Empty();
    }
    auto ptr = new uint8_t[size];
    return std::shared_ptr<GData>(new GData(ptr, size, free_uint8_array));
}

std::shared_ptr<GData> GData::Zeroed(size_t size) {
    auto data = Uninitialized(size);
    if (size) {
        memset(data->data(), 0, data->size());
    }
    return data;
}

std::shared_ptr<GData> GData::Managed(const void* buffer, size_t size, GData::ReleaseProc proc) {
    return std::shared_ptr<GData>(new GData(buffer, size, std::move(proc)));
}

std::shared_ptr<GData> GData::Unmanaged(const void* buffer, size_t size) {
    return Managed(buffer, size, free_do_nothing);
}

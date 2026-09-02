/**
 *  Copyright 2025 Mike Reed
 */

#ifndef GData_DEFINED
#define GData_DEFINED

#include "GTypes.h"

#include <functional>

class GData : public std::enable_shared_from_this<GData> {
public:
    ~GData() { m_releaseProc((void*)m_data); }

    size_t size() const { return m_size; }

    void* data() { return (void*)m_data; }
    const void* data() const { return m_data; }

    using ReleaseProc = std::function<void(void*)>;

    static std::shared_ptr<GData> Empty();
    static std::shared_ptr<GData> Uninitialized(size_t size);
    static std::shared_ptr<GData> Zeroed(size_t size);
    static std::shared_ptr<GData> Managed(const void* buffer, size_t size, ReleaseProc);
    static std::shared_ptr<GData> Unmanaged(const void* buffer, size_t size);
    static std::shared_ptr<GData> Copy(const void* src, size_t size);

private:
    ReleaseProc m_releaseProc;
    const void* m_data;
    size_t      m_size;

    GData(const void*, size_t, ReleaseProc);
};

#endif

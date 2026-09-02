/**
 *  Copyright 2026 Mike Reed
 */

#ifndef GRegistrant_DEFINED
#define GRegistrant_DEFINED

#include "../include/GTypes.h"
#include <algorithm>
#include <functional>

template <typename T> class GRegistrant {
    static GRegistrant<T>* gHead;

    GRegistrant* m_next;
    T            m_value;
public:
    GRegistrant(const T& value) : m_value(value) {
        m_next = gHead;
        gHead = this;
    }

    GRegistrant* next() const { return m_next; };
    const T& value() const { return m_value; }
    T& value() { return m_value; }

    static GRegistrant* Head() { return gHead; }

    static void Visit(const std::function<void(const T&)>& visitor) {
        for (auto reg = Head(); reg != nullptr; reg = reg->next()) {
            visitor(reg->value());
        }
    }

    static void Visit(const std::function<void(const T&, size_t)>& visitor) {
        size_t index = 0;
        for (auto reg = Head(); reg != nullptr; reg = reg->next()) {
            visitor(reg->value(), index);
            index += 1;
        }
    }

    static void Sort(const std::function<bool(const T& a, const T& b)>& pred) {
        std::vector<GRegistrant*> array;

        for (auto reg = Head(); reg != nullptr; reg = reg->next()) {
            array.push_back(reg);
        }

        std::sort(array.begin(), array.end(), [pred](const GRegistrant* a, const GRegistrant* b) {
            return pred(a->value(), b->value());
        });

        gHead = nullptr;
        for (size_t i = array.size(); i --> 0; ) {
            array[i]->m_next = gHead;
            gHead = array[i];
        }
    }
};

template <typename T> GRegistrant<T>* GRegistrant<T>::gHead;
// client must instantiate their global instance, like this:
// template ToolbarRegistrant* ToolbarRegistrant::gHead;

#define G_MACRO_CONCAT(X, Y)        G_MACRO_CONCAT_IMPL(X, Y)
#define G_MACRO_CONCAT_IMPL(X, Y)   X ## Y
#define G_MACRO_UNIQUE_NAME(name)   G_MACRO_CONCAT(name, __COUNTER__)
// Use this to create a custom macro for registering instances of your specialization, e.g.
//    #define REGISTER_TOOLBAR(order, fact) \
//        static ToolbarRegistrant G_MACRO_UNIQUE_NAME(toolbar_proc)({order, fact})

#endif

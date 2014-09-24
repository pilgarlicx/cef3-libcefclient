/**
 * @file v8_util.h
 *
 * @breif Misc. functions/macros to help working with CefV8XXX
 */
#ifndef _HENAN_TI_PLATFORM_V8_UTIL_H
#define _HENAN_TI_PLATFORM_V8_UTIL_H

#include <include/cef_process_message.h>
#include <include/cef_v8.h>
#include <include/cef_values.h>

namespace util {

void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target);
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target);
void SetList(const CefV8ValueList& source, CefRefPtr<CefListValue> target);
void SetList(CefRefPtr<CefListValue> source, CefV8ValueList& target);
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefListValue> target);

}

#endif // _HENAN_TI_PLATFORM_V8_UTIL_H

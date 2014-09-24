/**
 * @file v8_util.cpp
 *
 * @breif Impl of v8_util.h
 */
#include "v8_util.h"

#include "util.h"

namespace util {

namespace {

    // Transfer a V8 value to a List index.
    void SetListValue(CefRefPtr<CefListValue> list, int index,
                      CefRefPtr<CefV8Value> value) {
        if (value->IsArray()) {
            CefRefPtr<CefListValue> new_list = CefListValue::Create();
            SetList(value, new_list);
            list->SetList(index, new_list);
        } else if (value->IsString()) {
            list->SetString(index, value->GetStringValue());
        } else if (value->IsBool()) {
            list->SetBool(index, value->GetBoolValue());
        } else if (value->IsInt()) {
            list->SetInt(index, value->GetIntValue());
        } else if (value->IsDouble()) {
            list->SetDouble(index, value->GetDoubleValue());
        }
    }

    CefRefPtr<CefV8Value> ListValueToV8Value(CefRefPtr<CefListValue> value, int index)
    {
        CefRefPtr<CefV8Value> new_value;

        CefValueType type = value->GetType(index);
        switch (type) {
            case VTYPE_LIST: {
                CefRefPtr<CefListValue> list = value->GetList(index);
                new_value = CefV8Value::CreateArray(list->GetSize());
                SetList(list, new_value);
            } break;
            case VTYPE_BOOL:
            new_value = CefV8Value::CreateBool(value->GetBool(index));
            break;
            case VTYPE_DOUBLE:
            new_value = CefV8Value::CreateDouble(value->GetDouble(index));
            break;
            case VTYPE_INT:
            new_value = CefV8Value::CreateInt(value->GetInt(index));
            break;
            case VTYPE_STRING:
            new_value = CefV8Value::CreateString(value->GetString(index));
            break;
            default:
            new_value = CefV8Value::CreateNull();
            break;
        }

        return new_value;
    }

    // Transfer a List value to a V8 array index.
    void SetListValue(CefRefPtr<CefV8Value> list, int index,
                      CefRefPtr<CefListValue> value) {
        CefRefPtr<CefV8Value> new_value;

        CefValueType type = value->GetType(index);
        switch (type) {
            case VTYPE_LIST: {
                CefRefPtr<CefListValue> list = value->GetList(index);
                new_value = CefV8Value::CreateArray(list->GetSize());
                SetList(list, new_value);
            } break;
            case VTYPE_BOOL:
            new_value = CefV8Value::CreateBool(value->GetBool(index));
            break;
            case VTYPE_DOUBLE:
            new_value = CefV8Value::CreateDouble(value->GetDouble(index));
            break;
            case VTYPE_INT:
            new_value = CefV8Value::CreateInt(value->GetInt(index));
            break;
            case VTYPE_STRING:
            new_value = CefV8Value::CreateString(value->GetString(index));
            break;
            default:
            break;
        }

        if (new_value.get()) {
            list->SetValue(index, new_value);
        } else {
            list->SetValue(index, CefV8Value::CreateNull());
        }
    }

    // Transfer a List value to a V8 array index.
    void SetListValue(CefRefPtr<CefListValue> target, int index,
                      CefRefPtr<CefListValue> source) {
        CefValueType type = source->GetType(index);
        switch (type) {
            case VTYPE_LIST:
            target->SetList(index, source->GetList(index));
            break;
            case VTYPE_BOOL:
            target->SetBool(index, source->GetBool(index));
            break;
            case VTYPE_DOUBLE:
            target->SetDouble(index, source->GetDouble(index));
            break;
            case VTYPE_INT:
            target->SetInt(index, source->GetInt(index));
            break;
            case VTYPE_STRING:
            target->SetString(index, source->GetString(index));
            break;
            default:
            break;
        }
    }

}

// Transfer a V8 array to a List.
void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target) {
    ASSERT(source->IsArray());

    int arg_length = source->GetArrayLength();
    if (arg_length == 0)
        return;

    // Start with null types in all spaces.
    target->SetSize(arg_length);

    for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source->GetValue(i));
}

// Transfer a V8 vector to a list
void SetList(const CefV8ValueList& source, CefRefPtr<CefListValue> target)
{
    int arg_length = source.size();
    if (arg_length == 0)
        return;
    target->SetSize(arg_length);
    for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source[i]);
}

// Transfer a List to a V8 array.
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target) {
    ASSERT(target->IsArray());

    int arg_length = source->GetSize();
    if (arg_length == 0)
        return;

    for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source);
}

// Transfer a list to V8 vector
void SetList(CefRefPtr<CefListValue> source, CefV8ValueList& target)
{
    int arg_length = source->GetSize();
    if (arg_length == 0)
        return;
    target.resize(arg_length);
    for (int i = 0; i < arg_length; ++i)
        target[i] = ListValueToV8Value(source, i);
}

// Copy a list to another list
void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefListValue> target)
{
    int arg_length = source->GetSize();
    if (arg_length == 0)
        return;
    target->SetSize(arg_length);
    for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source);
}

}

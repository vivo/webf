/*
 * Copyright (C) 2019 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#include <cassert>
#include <js_error.h>
#include <js_type.h>

namespace alibaba {
namespace jsa {
namespace {

// This is used for generating short exception strings.
std::string kindToString(const Value &v, JSContext *rt = nullptr) {
  if (v.isUndefined()) {
    return "undefined";
  } else if (v.isNull()) {
    return "null";
  } else if (v.isBool()) {
    return v.getBool() ? "true" : "false";
  } else if (v.isNumber()) {
    return "a number";
  } else if (v.isString()) {
    return "a string";
  } else if (v.isSymbol()) {
    return "a symbol";
  } else {
    assert(v.isObject() && "Expecting object.");
    return rt != nullptr && v.getObject(*rt).isFunction(*rt) ? "a function"
                                                             : "an object";
  }
}

} // namespace

namespace detail {

void throwJSError(JSContext &rt, const char *msg) { throw JSError(rt, msg); }
} // namespace detail

Pointer &Pointer::operator=(Pointer &&other) {
  if (ptr_) {
    ptr_->invalidate();
  }
  ptr_ = other.ptr_;
  other.ptr_ = nullptr;
  return *this;
}

Object Object::getPropertyAsObject(JSContext &runtime, const char *name) const {
  Value v = getProperty(runtime, name);

  if (!v.isObject()) {
    throw JSError(runtime, std::string("getPropertyAsObject: property '") +
                               name + "' is " + kindToString(v, &runtime) +
                               ", expected an Object");
  }

  return v.getObject(runtime);
}

Function Object::getPropertyAsFunction(JSContext &runtime,
                                       const char *name) const {
  Object obj = getPropertyAsObject(runtime, name);
  if (!obj.isFunction(runtime)) {
    throw JSError(runtime, std::string("getPropertyAsFunction: property '") +
                               name + "' is " +
                               kindToString(std::move(obj), &runtime) +
                               ", expected a Function");
  };

  JSContext::PointerValue *value = obj.ptr_;
  obj.ptr_ = nullptr;
  return Function(value);
}

Array Object::asArray(JSContext &runtime) const & {
  if (!isArray(runtime)) {
    throw JSError(runtime, "Object is " +
                               kindToString(Value(runtime, *this), &runtime) +
                               ", expected an array");
  }
  return getArray(runtime);
}

Array Object::asArray(JSContext &runtime) && {
  if (!isArray(runtime)) {
    throw JSError(runtime, "Object is " +
                               kindToString(Value(runtime, *this), &runtime) +
                               ", expected an array");
  }
  return std::move(*this).getArray(runtime);
}

Function Object::asFunction(JSContext &runtime) const & {
  if (!isFunction(runtime)) {
    throw JSError(runtime, "Object is " +
                               kindToString(Value(runtime, *this), &runtime) +
                               ", expected a function");
  }
  return getFunction(runtime);
}

Function Object::asFunction(JSContext &runtime) && {
  if (!isFunction(runtime)) {
    throw JSError(runtime, "Object is " +
                               kindToString(Value(runtime, *this), &runtime) +
                               ", expected a function");
  }
  return std::move(*this).getFunction(runtime);
}

Value::Value(Value &&other) : Value(other.kind_) {
  if (kind_ == BooleanKind) {
    data_.boolean = other.data_.boolean;
  } else if (kind_ == NumberKind) {
    data_.number = other.data_.number;
  } else if (kind_ >= PointerKind) {
    new (&data_.pointer) Pointer(std::move(other.data_.pointer));
  }
  // when the other's dtor runs, nothing will happen.
  other.kind_ = UndefinedKind;
}

Value::Value(JSContext &runtime, const Value &other) : Value(other.kind_) {
  // data_ is uninitialized, so use placement new to create non-POD
  // types in it.  Any other kind of initialization will call a dtor
  // first, which is incorrect.
  if (kind_ == BooleanKind) {
    data_.boolean = other.data_.boolean;
  } else if (kind_ == NumberKind) {
    data_.number = other.data_.number;
  } else if (kind_ == SymbolKind) {
    // 在预分配内存中创建对象
    new (&data_.pointer) Pointer(runtime.cloneSymbol(other.data_.pointer.ptr_));
  } else if (kind_ == StringKind) {
    new (&data_.pointer) Pointer(runtime.cloneString(other.data_.pointer.ptr_));
  } else if (kind_ >= ObjectKind) {
    new (&data_.pointer) Pointer(runtime.cloneObject(other.data_.pointer.ptr_));
  }
}

Value::~Value() {
  if (kind_ >= PointerKind) {
    data_.pointer.~Pointer();
  }
}

Value Value::createFromJsonUtf8(JSContext &runtime, const uint8_t *json,
                                size_t length) {
  Function parseJson = runtime.global()
                           .getPropertyAsObject(runtime, "JSON")
                           .getPropertyAsFunction(runtime, "parse");
  return parseJson.call(runtime, String::createFromUtf8(runtime, json, length));
}

bool Value::strictEquals(JSContext &runtime, const Value &a, const Value &b) {
  if (a.kind_ != b.kind_) {
    return false;
  }
  switch (a.kind_) {
  case UndefinedKind:
  case NullKind:
    return true;
  case BooleanKind:
    return a.data_.boolean == b.data_.boolean;
  case NumberKind:
    return a.data_.number == b.data_.number;
  case SymbolKind:
    return runtime.strictEquals(static_cast<const Symbol &>(a.data_.pointer),
                                static_cast<const Symbol &>(b.data_.pointer));
  case StringKind:
    return runtime.strictEquals(static_cast<const String &>(a.data_.pointer),
                                static_cast<const String &>(b.data_.pointer));
  case ObjectKind:
    return runtime.strictEquals(static_cast<const Object &>(a.data_.pointer),
                                static_cast<const Object &>(b.data_.pointer));
  }
  return false;
}

double Value::asNumber() const {
  if (!isNumber()) {
    throw JSANativeException("Value is " + kindToString(*this) +
                             ", expected a number");
  }

  return getNumber();
}

Object Value::asObject(JSContext &rt) const & {
  if (!isObject()) {
    throw JSError(rt, "Value is " + kindToString(*this, &rt) +
                          ", expected an Object");
  }

  return getObject(rt);
}

Object Value::asObject(JSContext &rt) && {
  if (!isObject()) {
    throw JSError(rt, "Value is " + kindToString(*this, &rt) +
                          ", expected an Object");
  }
  auto ptr = data_.pointer.ptr_;
  data_.pointer.ptr_ = nullptr;
  return static_cast<Object>(ptr);
}

Symbol Value::asSymbol(JSContext &rt) const & {
  if (!isSymbol()) {
    throw JSError(rt, "Value is " + kindToString(*this, &rt) +
                          ", expected a Symbol");
  }

  return getSymbol(rt);
}

Symbol Value::asSymbol(JSContext &rt) && {
  if (!isSymbol()) {
    throw JSError(rt, "Value is " + kindToString(*this, &rt) +
                          ", expected a Symbol");
  }

  return std::move(*this).getSymbol(rt);
}

String Value::asString(JSContext &rt) const & {
  if (!isString()) {
    throw JSError(rt, "Value is " + kindToString(*this, &rt) +
                          ", expected a String");
  }

  return getString(rt);
}

String Value::asString(JSContext &rt) && {
  if (!isString()) {
    throw JSError(rt, "Value is " + kindToString(*this, &rt) +
                          ", expected a String");
  }

  return std::move(*this).getString(rt);
}

String Value::toString(JSContext &runtime) const {
  Function toString = runtime.global().getPropertyAsFunction(runtime, "String");
  return toString.call(runtime, *this).getString(runtime);
}

Array Array::createWithElements(JSContext &rt,
                                std::initializer_list<Value> elements) {
  Array result(rt, elements.size());
  size_t index = 0;
  for (const auto &element : elements) {
    result.setValueAtIndex(rt, index++, element);
  }
  return result;
}

} // namespace jsa
} // namespace alibaba

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/java/java_method.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/memory/singleton.h"
#include "base/string_util.h"  // For ReplaceSubstringsAfterOffset

using base::android::AttachCurrentThread;
using base::android::ConvertJavaStringToUTF8;
using base::android::MethodID;
using base::android::ScopedJavaLocalRef;

namespace {

// Java's reflection API represents types as a string using an extended 'binary
// name'. This converts to an enum which we store in place of the binary name
// for simplicity.
JavaType::Type BinaryNameToType(const std::string& binary_name) {
  if (binary_name == "boolean") {
    return JavaType::TypeBoolean;
  } else if (binary_name == "byte") {
    return JavaType::TypeByte;
  } else if (binary_name == "char") {
    return JavaType::TypeChar;
  } else if (binary_name == "short") {
    return JavaType::TypeShort;
  } else if (binary_name == "int") {
    return JavaType::TypeInt;
  } else if (binary_name == "long") {
    return JavaType::TypeLong;
  } else if (binary_name == "float") {
    return JavaType::TypeFloat;
  } else if (binary_name == "double") {
    return JavaType::TypeDouble;
  } else if (binary_name == "void") {
    return JavaType::TypeVoid;
  } else if (binary_name[0] == '[') {
    return JavaType::TypeArray;
  } else if (binary_name == "java.lang.String") {
    return JavaType::TypeString;
  }
  return JavaType::TypeObject;
}

std::string BinaryNameToJNIName(const std::string& binary_name,
                                JavaType::Type* type) {
  DCHECK(type);
  *type = BinaryNameToType(binary_name);
  switch (*type) {
    case JavaType::TypeBoolean:
      return "Z";
    case JavaType::TypeByte:
      return "B";
    case JavaType::TypeChar:
      return "C";
    case JavaType::TypeShort:
      return "S";
    case JavaType::TypeInt:
      return "I";
    case JavaType::TypeLong:
      return "J";
    case JavaType::TypeFloat:
      return "F";
    case JavaType::TypeDouble:
      return "D";
    case JavaType::TypeVoid:
      return "V";
    case JavaType::TypeArray:
      return "[";
    default:
      DCHECK (*type == JavaType::TypeString || *type == JavaType::TypeObject);
      std::string jni_name = "L" + binary_name + ";";
      ReplaceSubstringsAfterOffset(&jni_name, 0, ".", "/");
      return jni_name;
  }
}

class MethodGetParameterTypesID : public MethodID {
 public:
  static MethodGetParameterTypesID* GetInstance() {
    return Singleton<MethodGetParameterTypesID>::get();
  }
 private:
  friend struct DefaultSingletonTraits<MethodGetParameterTypesID>;
  MethodGetParameterTypesID()
      : MethodID(AttachCurrentThread(), "java/lang/reflect/Method",
                 "getParameterTypes", "()[Ljava/lang/Class;") {
  }
  DISALLOW_COPY_AND_ASSIGN(MethodGetParameterTypesID);
};

class MethodGetNameID : public MethodID {
 public:
  static MethodGetNameID* GetInstance() {
    return Singleton<MethodGetNameID>::get();
  }
 private:
  friend struct DefaultSingletonTraits<MethodGetNameID>;
  MethodGetNameID()
      : MethodID(AttachCurrentThread(), "java/lang/reflect/Method",
                 "getName", "()Ljava/lang/String;") {
  }
  DISALLOW_COPY_AND_ASSIGN(MethodGetNameID);
};

class MethodGetReturnTypeID : public MethodID {
 public:
  static MethodGetReturnTypeID* GetInstance() {
    return Singleton<MethodGetReturnTypeID>::get();
  }
 private:
  friend struct DefaultSingletonTraits<MethodGetReturnTypeID>;
  MethodGetReturnTypeID()
      : MethodID(AttachCurrentThread(), "java/lang/reflect/Method",
                 "getReturnType", "()Ljava/lang/Class;") {
  }
  DISALLOW_COPY_AND_ASSIGN(MethodGetReturnTypeID);
};

class MethodGetDeclaringClassID : public MethodID {
 public:
  static MethodGetDeclaringClassID* GetInstance() {
    return Singleton<MethodGetDeclaringClassID>::get();
  }
 private:
  friend struct DefaultSingletonTraits<MethodGetDeclaringClassID>;
  MethodGetDeclaringClassID()
      : MethodID(AttachCurrentThread(), "java/lang/reflect/Method",
                 "getDeclaringClass", "()Ljava/lang/Class;") {
  }
  DISALLOW_COPY_AND_ASSIGN(MethodGetDeclaringClassID);
};

class ClassGetNameID : public MethodID {
 public:
  static ClassGetNameID* GetInstance() {
    return Singleton<ClassGetNameID>::get();
  }
 private:
  friend struct DefaultSingletonTraits<ClassGetNameID>;
  ClassGetNameID()
      : MethodID(AttachCurrentThread(), "java/lang/Class", "getName",
                 "()Ljava/lang/String;") {
  }
  DISALLOW_COPY_AND_ASSIGN(ClassGetNameID);
};

}  // namespace

JavaMethod::JavaMethod(const base::android::JavaRef<jobject>& method)
    : java_method_(method),
      have_calculated_num_parameters_(false),
      id_(NULL) {
  JNIEnv* env = java_method_.env();
  // On construction, we do nothing except get the name. Everything else is
  // done lazily.
  ScopedJavaLocalRef<jstring> name(env, static_cast<jstring>(
      env->CallObjectMethod(java_method_.obj(),
                            MethodGetNameID::GetInstance()->id())));
  name_ = ConvertJavaStringToUTF8(env, name.obj());
}

JavaMethod::~JavaMethod() {
}

size_t JavaMethod::num_parameters() const {
  EnsureNumParametersIsSetUp();
  return num_parameters_;
}

JavaType::Type JavaMethod::parameter_type(size_t index) const {
  EnsureTypesAndIDAreSetUp();
  return parameter_types_[index];
}

JavaType::Type JavaMethod::return_type() const {
  EnsureTypesAndIDAreSetUp();
  return return_type_;
}

jmethodID JavaMethod::id() const {
  EnsureTypesAndIDAreSetUp();
  return id_;
}

void JavaMethod::EnsureNumParametersIsSetUp() const {
  if (have_calculated_num_parameters_) {
    return;
  }
  have_calculated_num_parameters_ = true;

  // The number of parameters will be used frequently when determining
  // whether to call this method. We don't get the ID etc until actually
  // required.
  JNIEnv* env = java_method_.env();
  ScopedJavaLocalRef<jarray> parameters(env, static_cast<jarray>(
      env->CallObjectMethod(java_method_.obj(),
                            MethodGetParameterTypesID::GetInstance()->id())));
  num_parameters_ = env->GetArrayLength(parameters.obj());
}

void JavaMethod::EnsureTypesAndIDAreSetUp() const {
  if (id_) {
    return;
  }

  // Get the parameters
  JNIEnv* env = java_method_.env();
  ScopedJavaLocalRef<jobjectArray> parameters(env, static_cast<jobjectArray>(
      env->CallObjectMethod(java_method_.obj(),
                            MethodGetParameterTypesID::GetInstance()->id())));
  // Usually, this will already have been called.
  EnsureNumParametersIsSetUp();
  DCHECK_EQ(num_parameters_,
            static_cast<size_t>(env->GetArrayLength(parameters.obj())));

  // Java gives us the argument type using an extended version of the 'binary
  // name'. See
  // http://download.oracle.com/javase/1.4.2/docs/api/java/lang/Class.html#getName().
  // If we build the signature now, there's no need to store the binary name
  // of the arguments. We just store the simple type.
  std::string signature("(");

  // Form the signature and record the parameter types.
  parameter_types_.resize(num_parameters_);
  for (size_t i = 0; i < num_parameters_; ++i) {
    ScopedJavaLocalRef<jobject> parameter(env, env->GetObjectArrayElement(
        parameters.obj(), i));
    ScopedJavaLocalRef<jstring> name(env, static_cast<jstring>(
        env->CallObjectMethod(parameter.obj(),
                              ClassGetNameID::GetInstance()->id())));
    std::string name_utf8 = ConvertJavaStringToUTF8(env, name.obj());
    signature += BinaryNameToJNIName(name_utf8, &parameter_types_[i]);
  }
  signature += ")";

  // Get the return type
  ScopedJavaLocalRef<jclass> clazz(env, static_cast<jclass>(
      env->CallObjectMethod(java_method_.obj(),
                            MethodGetReturnTypeID::GetInstance()->id())));
  ScopedJavaLocalRef<jstring> name(env, static_cast<jstring>(
      env->CallObjectMethod(clazz.obj(), ClassGetNameID::GetInstance()->id())));
  signature += BinaryNameToJNIName(ConvertJavaStringToUTF8(env, name.obj()),
                                   &return_type_);

  // Get the ID for this method.
  ScopedJavaLocalRef<jclass> declaring_class(env, static_cast<jclass>(
      env->CallObjectMethod(java_method_.obj(),
                            MethodGetDeclaringClassID::GetInstance()->id())));
  id_ = base::android::GetMethodID(env, declaring_class.obj(), name_.c_str(),
                                   signature.c_str());

  java_method_.Reset();
}

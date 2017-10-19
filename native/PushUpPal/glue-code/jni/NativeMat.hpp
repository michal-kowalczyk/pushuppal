#pragma once

#include <opencv2/core/mat.hpp>
#include "djinni_support.hpp"

namespace cv {
namespace djinni {
namespace jni {

class NativeMat final : ::djinni::JniInterface<::cv::Mat, NativeMat> {
public:
    using CppType = ::cv::Mat;
    using JniType = jobject;

    using Boxed = NativeMat;

    ~NativeMat();

    static CppType toCpp(JNIEnv* jniEnv, JniType matObj) {
        auto matClass = jniEnv->GetObjectClass(matObj);
        jfieldID nativeObj = jniEnv->GetFieldID(matClass, "nativeObj", "J");
        long matPtr = jniEnv->GetLongField(matObj, nativeObj);
        ::cv::Mat& mat = *((::cv::Mat*)matPtr);
        return mat;
    }

//    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
//        construct the Java Mat with a pointer to the C++ Mat
//    }

private:
    NativeMat();
    friend ::djinni::JniClass<NativeMat>;
    friend ::djinni::JniInterface<::cv::Mat, NativeMat>;
};

}  // namespace jni
}  // namespace djinni
}  // namespace cv
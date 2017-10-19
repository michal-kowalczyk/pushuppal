#include <opencv2/opencv.hpp>
#import "OpenCvMat+Private.h"
#include <cassert>

@implementation OpenCvMatImpl

- (nonnull instancetype)initWithMat:(nonnull const cv::Mat*)mat
{
    if (self = [super init]) {
        _mat = mat;
    }
    return self;
}

@end


namespace cv
{
namespace djinni
{
namespace objc
{

auto CvMatConverter::toCpp(ObjcType objcMat) -> CppType
{
    assert(objcMat);
    return *static_cast<const CppType*>(static_cast<OpenCvMatImpl*>(objcMat).mat);
}

auto CvMatConverter::fromCpp(const CppType& cppMat) -> ObjcType
{
    const cv::Mat* cppMatPtr = &cppMat;
    return [[::OpenCvMatImpl alloc] initWithMat:cppMatPtr];
}

}  // namespace objc
}  // namespace djinni
}  // namespace cv

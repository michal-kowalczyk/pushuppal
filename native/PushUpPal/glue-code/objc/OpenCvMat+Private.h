#include <opencv2/core/core.hpp>
#import "OpenCvMat.h"

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@class OpenCvMat;

@interface OpenCvMatImpl : OpenCvMat
+ (nullable instancetype) new NS_UNAVAILABLE;
- (nullable instancetype) init NS_UNAVAILABLE;
- (nonnull instancetype)initWithMat:(nonnull const cv::Mat*)mat NS_DESIGNATED_INITIALIZER;

@property (nonatomic, readonly) const cv::Mat* mat;
@end

namespace cv
{
namespace djinni
{
namespace objc
{

struct CvMatConverter
{
    using CppType = cv::Mat;
    using ObjcType = OpenCvMat*;

    using Boxed = OpenCvMat;

    static CppType toCpp(ObjcType objc);
    static ObjcType fromCpp(const CppType& cpp);
};

}  // namespace objc
}  // namespace djinni
}  // namespace cv

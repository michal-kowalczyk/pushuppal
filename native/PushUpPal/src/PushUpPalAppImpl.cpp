#include "PushUpPalApp.hpp"

#include "FaceRecognitionAndOpticalFlowBasedPushUpDetector.h"

std::shared_ptr<generated::PushUpPalApp> generated::PushUpPalApp::create(generated::CameraOrientation orientation,
                                                                         const std::string& classifierFilePath)
{
    return std::make_shared<FaceRecognitionAndOpticalFlowBasedPushUpDetector>(orientation, classifierFilePath);
}
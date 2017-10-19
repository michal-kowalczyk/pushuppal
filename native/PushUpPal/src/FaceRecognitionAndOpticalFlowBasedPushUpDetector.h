//
// Created by Michal Kowalczyk on 09/10/2017.
//

#ifndef PUSHUPPAL_FACERECOGNITIONANDOPTICALFLOWBASEDPUSHUPDETECTOR_H
#define PUSHUPPAL_FACERECOGNITIONANDOPTICALFLOWBASEDPUSHUPDETECTOR_H

#include "PushUpPalApp.hpp"

#include <memory>

class FaceRecognitionAndOpticalFlowBasedPushUpDetector : public generated::PushUpPalApp {
public:
    FaceRecognitionAndOpticalFlowBasedPushUpDetector(generated::CameraOrientation orientation,
                                                     const std::string& classifierFilePath);

    ~FaceRecognitionAndOpticalFlowBasedPushUpDetector() override;

    void onFrame(const ::cv::Mat& frame) override;

    void start() override;

    void stop() override;

    bool isStarted() override;

    void reset() override;

    void setListener(const std::shared_ptr<generated::PushUpListener>& listener) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif //PUSHUPPAL_FACERECOGNITIONANDOPTICALFLOWBASEDPUSHUPDETECTOR_H

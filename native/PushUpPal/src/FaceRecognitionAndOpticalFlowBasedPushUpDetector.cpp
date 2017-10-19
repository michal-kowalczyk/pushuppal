//
// Created by Michal Kowalczyk on 09/10/2017.
//

#include <vector>

#include <opencv2/opencv.hpp>

#include <CameraOrientation.hpp>

#include "PushUpListener.hpp"
#include "TimeSerie.h"
#include "FaceRecognitionAndOpticalFlowBasedPushUpDetector.h"
#include "ConditionsObserver.h"

class CameraOrientionPointConverter
{
public:
    CameraOrientionPointConverter(generated::CameraOrientation orientation)
    : cameraOrientation_(orientation)
    {
        // do nothing
    }

    cv::Point convertPoint(const cv::Point& point) const
    {
        if (generated::CameraOrientation::PORTRAIT == cameraOrientation_)
        {
            return point;
        }
        return cv::Point(point.y, point.x);
    }

private:
    const generated::CameraOrientation cameraOrientation_;
};

struct ComparisonResult
{
    ComparisonResult()
            : keptPoints(0)
    {
        // do nothing
    }

    int keptPoints;
};

struct PushUpData
{
    ComparisonResult lastComparisonResult;
    cv::Rect face;
    cv::Rect lastNonEmptyFace;
    TimeSerie<int, 2> lastFewKeptPoints;
};

class RiseObserver : public ConditionsObserver
{
public:
    RiseObserver(const PushUpData& data)
            : data_(data)
    {
        // do nothing
    }

    bool areConditionsMet() override
    {
        return data_.lastFewKeptPoints.min() > 15 &&
               !data_.face.empty();
    }

private:
    const PushUpData& data_;
};

class LowerObserver : public ConditionsObserver
{
public:
    LowerObserver(const PushUpData& data)
            : data_(data)
    {
        // do nothing
    }

    bool areConditionsMet() override
    {
        return data_.lastFewKeptPoints.max() < 10 &&
               data_.face.empty();
    }

private:
    const PushUpData& data_;
};

class MeaningfulPointsWithinRect
{
public:
    explicit MeaningfulPointsWithinRect(CameraOrientionPointConverter orientationPointConverter)
            : orientationPointConverter_(orientationPointConverter)
            , termcrit_(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03)
            , maxPoints_(50)
    {
        // do nothing
    }

    ComparisonResult processFrame(cv::Mat frameGray)
    {
        ComparisonResult result;

        frameGray_ = frameGray.clone();

        if (lastRect_.empty())
        {
            return result;
        }
        if (!prevPoints_.empty())
        {
            trackPoints();
            result.keptPoints = currPoints_.size();
        }

        std::swap(prevPoints_, currPoints_);
        cv::swap(prevFrameGray_, frameGray_);

        return result;
    }

    ComparisonResult processFrame(cv::Mat frameGray, const cv::Rect& rect)
    {
        assert(!rect.empty());

        ComparisonResult result;

        frameGray_ = frameGray.clone();
        lastRect_ = rect;

        if (prevPoints_.empty())
        {
            findNewPoints();
        }
        else
        {
            trackPoints();
            result.keptPoints = currPoints_.size();

            findNewPoints();
        }

        std::swap(prevPoints_, currPoints_);
        cv::swap(prevFrameGray_, frameGray_);

        return result;
    }

    void drawPoints(cv::Mat frame, const float scale)
    {
        for (const auto& point : prevPoints_)
        {
            cv::circle(frame,
                       orientationPointConverter_.convertPoint(point * scale),
                       3,
                       cv::Scalar(0, 255, 0), -1, 8);
        }
    }

private:
    void findNewPoints()
    {
        if (maxPoints_ <= currPoints_.size())
        {
            return;
        }
        static const cv::Size subPixWinSize(10, 10);

        auto smallerRect = lastRect_;
        int hMargin = smallerRect.width / 5;
        int vMargin = smallerRect.height / 5;
        smallerRect.x += hMargin;
        smallerRect.width -= hMargin * 2;
        smallerRect.y += vMargin;
        smallerRect.height -= vMargin * 2;

        nextPoints_.clear();

        cv::Mat smallGray = frameGray_(smallerRect);
        cv::goodFeaturesToTrack(smallGray, nextPoints_, maxPoints_ - currPoints_.size(), 0.01,
                                10);
        if (nextPoints_.empty())
        {
            return;
        }
        cv::cornerSubPix(smallGray, nextPoints_, subPixWinSize, cv::Size(-1, -1), termcrit_);

        for (const auto &point : nextPoints_)
        {
            currPoints_.emplace_back(point.x + smallerRect.x, point.y + smallerRect.y);
        }
    }

    void trackPoints()
    {
        if (prevFrameGray_.empty())
        {
            return;
        }

        static const cv::Size winSize(31, 31);

        std::vector<uchar> status;
        std::vector<float> err;

        currPoints_.clear();
        cv::calcOpticalFlowPyrLK(prevFrameGray_, frameGray_, prevPoints_, currPoints_, status, err, winSize, 3, termcrit_, 0, 0.001);
        size_t i, k;
        for (i = 0, k = 0; i < currPoints_.size(); i++)
        {
            if (!status[i] || (!lastRect_.empty() && !lastRect_.contains(currPoints_[i])))
            {
                continue;
            }
            currPoints_[k] = currPoints_[i];
            ++k;
        }
        currPoints_.resize(k);
    }

private:
    const CameraOrientionPointConverter orientationPointConverter_;
    const cv::TermCriteria termcrit_;
    const int maxPoints_;
    cv::Mat frameGray_;
    cv::Mat prevFrameGray_;
    cv::Rect lastRect_;
    std::vector<cv::Point2f> prevPoints_;
    std::vector<cv::Point2f> currPoints_;
    std::vector<cv::Point2f> nextPoints_;
};

cv::Rect detectFace(cv::CascadeClassifier& faceCascade, cv::Mat frameGray)
{
    static constexpr double kScaleFactor = 1.1;
    static constexpr int kMinNeighbors = 10;
    const cv::Size minSize(frameGray.cols / 6, frameGray.rows / 6);

    cv::Rect biggestFace(0, 0, 0, 0);
    auto biggestArea = biggestFace.area();
    std::vector<cv::Rect> faces;

    faceCascade.detectMultiScale(frameGray, faces, kScaleFactor, kMinNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, minSize);
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        auto currentArea = faces[i].area();
        if (biggestArea < currentArea)
        {
            biggestArea = currentArea;
            biggestFace = faces[i];
        }
    }

    return biggestFace;
}

class FaceRecognitionAndOpticalFlowBasedPushUpDetector::Impl
{
public:
    Impl(generated::CameraOrientation orientation,
         const std::string& classifierFilePath)
            : cameraOrientation_(orientation)
            , orientationPointConverter_(orientation)
            , pointsWithinRect_(orientationPointConverter_)
            , riseObserver_(data_)
            , lowerObserver_(data_)
            , currentObserver_(&lowerObserver_)
            , isStarted_(false)
            , reps_(0)
    {
        if (!faceCascade_.load(classifierFilePath))
        {
            throw std::runtime_error("Couldn't load classifier file from " + classifierFilePath);
        }
    }

    void onFrame(const ::cv::Mat& frame)
    {
        const float scale = static_cast<float>(std::min(frame.cols, frame.rows)) / 160.0f;
        cv::resize(frame, smallFrame, cv::Size(frame.cols / scale, frame.rows / scale));
        cvtColor(smallFrame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayFrame, grayFrame);

        cv::Mat grayFrameForFaceRecogniction = grayFrame;
        if (generated::CameraOrientation::LANDSCAPE == cameraOrientation_)
        {
            cv::transpose(grayFrame, grayTransposedFrame);
            grayFrameForFaceRecogniction = grayTransposedFrame;
        }
        data_.face = detectFace(faceCascade_, grayFrameForFaceRecogniction);

        if (!data_.face.empty())
        {
            data_.lastNonEmptyFace = data_.face;
            data_.lastComparisonResult = pointsWithinRect_.processFrame(grayFrameForFaceRecogniction, data_.face);
        }
        else
        {
            data_.lastComparisonResult = pointsWithinRect_.processFrame(grayFrameForFaceRecogniction);
        }

        data_.lastFewKeptPoints.addValue(data_.lastComparisonResult.keptPoints);

        drawDebugInformation(frame, scale);

        if (isStarted_)
        {
            if (!currentObserver_->areConditionsMet())
            {
                return;
            }
            if (currentObserver_ == &lowerObserver_)
            {
                currentObserver_ = &riseObserver_;
            }
            else
            {
                currentObserver_ = &lowerObserver_;
                ++reps_;
                if (listener_)
                {
                    listener_->onPushUp(reps_);
                }
            }
        }
    }

    void start()
    {
        isStarted_ = true;
        currentObserver_ = &lowerObserver_;
    }

    void stop()
    {
        isStarted_ = false;
    }

    bool isStarted()
    {
        return isStarted_;
    }

    void reset()
    {
        isStarted_ = false;
        reps_ = 0;
        if (listener_)
        {
            listener_->onPushUp(reps_);
        }
    }

    void setListener(const std::shared_ptr<generated::PushUpListener>& listener)
    {
        listener_ = listener;
    }

private:
    void drawDebugInformation(const cv::Mat& frame, const float scale)
    {
        pointsWithinRect_.drawPoints(frame, scale);

        if (data_.face.empty())
        {
            cv::rectangle(frame,
                          orientationPointConverter_.convertPoint(data_.lastNonEmptyFace.tl() * scale),
                          orientationPointConverter_.convertPoint(data_.lastNonEmptyFace.br() * scale),
                          cv::Scalar(0, 0, 255));
        }
        else
        {
            cv::rectangle(frame,
                          orientationPointConverter_.convertPoint(data_.face.tl() * scale),
                          orientationPointConverter_.convertPoint(data_.face.br() * scale),
                          cv::Scalar(0, 255, 0));
        }
    }

private:
    const generated::CameraOrientation cameraOrientation_;
    const CameraOrientionPointConverter orientationPointConverter_;
    MeaningfulPointsWithinRect pointsWithinRect_;
    cv::CascadeClassifier faceCascade_;
    PushUpData data_;
    RiseObserver riseObserver_;
    LowerObserver lowerObserver_;
    ConditionsObserver* currentObserver_;
    std::shared_ptr<generated::PushUpListener> listener_;
    bool isStarted_;
    int reps_;
    cv::Mat smallFrame;
    cv::Mat grayFrame;
    cv::Mat grayTransposedFrame;
};

FaceRecognitionAndOpticalFlowBasedPushUpDetector::FaceRecognitionAndOpticalFlowBasedPushUpDetector(generated::CameraOrientation orientation,
                                                                                                   const std::string& classifierFilePath)
: impl_(new Impl(orientation, classifierFilePath))
{
    // do nothing
}

FaceRecognitionAndOpticalFlowBasedPushUpDetector::~FaceRecognitionAndOpticalFlowBasedPushUpDetector()
{
    // do nothing
}

void FaceRecognitionAndOpticalFlowBasedPushUpDetector::onFrame(const ::cv::Mat& frame)
{
    impl_->onFrame(frame);
}

void FaceRecognitionAndOpticalFlowBasedPushUpDetector::start()
{
    impl_->start();
}

void FaceRecognitionAndOpticalFlowBasedPushUpDetector::stop()
{
    impl_->stop();
}

bool FaceRecognitionAndOpticalFlowBasedPushUpDetector::isStarted()
{
    return impl_->isStarted();
}

void FaceRecognitionAndOpticalFlowBasedPushUpDetector::reset()
{
    impl_->reset();
}

void FaceRecognitionAndOpticalFlowBasedPushUpDetector::setListener(const std::shared_ptr<generated::PushUpListener>& listener)
{
    impl_->setListener(listener);
}

#ifndef INCLUDE_CAMERA_MOCK_HPP_
#define INCLUDE_CAMERA_MOCK_HPP_

#include <memory>

#include "camera/interface.hpp"

class MockCamera : public CameraInterface {
 public:
    explicit MockCamera(CameraConfiguration config);
    ~MockCamera() = default;
    void connect() override;
    bool verifyConnection() override;
    void takePicture() override;
    ImageData getLastPicture() override;
    bool takePictureForSeconds(int sec) override;
    void startTakingPictures(double intervalSec) override;
    bool isDoneTakingPictures() override;

 private:
    std::unique_ptr<ImageData> lastPicture;
};

#endif  // INCLUDE_CAMERA_MOCK_HPP_

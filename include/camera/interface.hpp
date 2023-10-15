#ifndef CAMERA_INTERFACE_HPP_
#define CAMERA_INTERFACE_HPP_
#include <nlohmann/json.hpp>
// TODO: import OpenCV library here

/*
 * FYI: this is class that will standardize image data but
 * if all of our cameras have a uniform image output type
 * this class is completely and utterly useless.
 *                                      - Boris
 *
 * We will also need to develop a custom converter
 *                                      - Boris (10/11)
 */
class ImageData
{
private:
    String name final;
    String path final;
    Mat data final;

public:
    String getName();
    String getPath();
    Mat getData();
}

// ? possibly convert most common / important json fields to
// ? data fields
class CameraConfiguration
{
private:
    json configJson;

public:
    CameraConfiguration(json config);

    void updateConfig(json newSetting);

    void updateConfigField(String key, T value);

    json getConfig();

    json getConfigField();
};

class CameraInterface
{
private:
    CameraConfiguration config
        ImageData recentPicture; // might need to move it to public
    bool doneTakingPicture;      // overengineering time
    String uploadPath;
    // Interpreter interp
    // TODO: SERVER CONNECTION HERE ?

    void imgConvert();

public:
    CameraInterface(CameraConfiguration config);

    void connect();

    bool verifyConnection();

    void takePicture();

    ImageData getLastPicture();

    bool takePictureForSeconds(int sec);

    void startTakingPictures(double intervalSec);

    bool isDoneTakingPictures();

    CameraConfiguration getConfig();

    void updateConfig(CameraConfiguration newConfig);

    void updateConfig(json newJsonConfig);

    String getUploadPath();

    void setUploadPath(String path);

    void uploadPicture(ImageData img);

    vector<ImageData> listPicturesFromUploadPath();

    ImageData getImageByName(String name);

    // server connection methods here
    // virtual methods for all possible camera actions
};

#endif // CAMERA_INTERFACE_HPP_
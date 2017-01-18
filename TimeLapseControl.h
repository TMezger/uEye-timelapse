#include "uEye.h"
#include <chrono>
#include <string>

const static unsigned int NUMBER_OF_IMAGE_MEM = 4;

class CTimeLapseControl
{
public:
    CTimeLapseControl();
    ~CTimeLapseControl();
    void Run();
    void Stop();
    void SetTimeBetweenCapturedImages(unsigned int timeBetweenCapturedImages_s);
    void SetImagePath(std::string path);

private:
    void InitCamera();
    bool isHistogramOK(int imageID);

    std::chrono::time_point<std::chrono::system_clock> m_lastTimeCaptured;
    bool m_useNextImage;
    bool m_initializationSuccessful;
    unsigned int m_timeBetweenImages_s;
    HIDS m_hCam;
    bool m_captureRunning;
    char * m_imageMem[NUMBER_OF_IMAGE_MEM];
    int m_imageMemID[NUMBER_OF_IMAGE_MEM];
    std::string m_path;
    IS_RECT m_imageSize;
    int m_colorMode;
};
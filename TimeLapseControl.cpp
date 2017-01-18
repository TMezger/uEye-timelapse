#include "TimeLapseControl.h"
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <ctime>

int getBitsPerPixelForColorformat(int colorFormat)
{
    switch(colorFormat)
    {
    case IS_CM_RGBA12_UNPACKED:
    case IS_CM_BGRA12_UNPACKED:
        return 64;
        break;

    case IS_CM_RGB12_UNPACKED:
    case IS_CM_BGR12_UNPACKED:
    case IS_CM_RGB10_UNPACKED:
    case IS_CM_BGR10_UNPACKED:
        return 48;
        break;

    case IS_CM_RGBA8_PACKED:
    case IS_CM_BGRA8_PACKED:
    case IS_CM_RGB10_PACKED:
    case IS_CM_BGR10_PACKED:
    case IS_CM_RGBY8_PACKED:
    case IS_CM_BGRY8_PACKED:
        return 32;
        break;

    case IS_CM_RGB8_PACKED:
    case IS_CM_BGR8_PACKED:
        return 24;
        break;

    case IS_CM_BGR565_PACKED:
    case IS_CM_UYVY_PACKED:
    case IS_CM_CBYCRY_PACKED:
        return 16;
        break;

    case IS_CM_BGR5_PACKED:
        return 15;
        break;

    case IS_CM_MONO16:
    case IS_CM_MONO12:
    case IS_CM_MONO10:
    case IS_CM_SENSOR_RAW16:
    case IS_CM_SENSOR_RAW12:
    case IS_CM_SENSOR_RAW10:
        return 16;
        break;

    case IS_CM_RGB8_PLANAR:
        return 24;
        break;

    case IS_CM_MONO8:
    case IS_CM_SENSOR_RAW8:
    default:
        return 8;
        break;
    }  
}

CTimeLapseControl::CTimeLapseControl()
{
    // Init members
    m_hCam = 0;
    m_timeBetweenImages_s = 3600; // one hour as default
    m_captureRunning = false;
    m_lastTimeCaptured = std::chrono::system_clock::now();
    m_useNextImage = false;
    m_initializationSuccessful = true;
    m_path = "BaucamImages/";
    m_colorMode = 0;
    m_imageSize = IS_RECT();

    // Init camera
    InitCamera();

    // Init image memory for an image queue
    int ret = 0;
    for (unsigned int i = 0; i < NUMBER_OF_IMAGE_MEM; ++i)
    {
        int ret = is_AllocImageMem (m_hCam, m_imageSize.s32Width, m_imageSize.s32Height,
            getBitsPerPixelForColorformat(m_colorMode), &m_imageMem[i], &m_imageMemID[i]);
        if (ret != IS_SUCCESS)
        {
            printf("Image memory allocation error: %d\n", ret);
            m_initializationSuccessful = false;
        }
        else
        {
            ret = is_AddToSequence(m_hCam, m_imageMem[i], m_imageMemID[i]);
            if (ret != IS_SUCCESS)
            {
                printf("Image memory add to sequence error: %d\n", ret);
                m_initializationSuccessful = false;
            }
        }
    }

    // Init the image queue
    ret = is_InitImageQueue(m_hCam, 0);
    if (ret != IS_SUCCESS)
    {
        printf("Init image queue error: %d\n", ret);
        m_initializationSuccessful = false;
    }
}

CTimeLapseControl::~CTimeLapseControl()
{
    // Exit image queue
    is_ExitImageQueue(m_hCam);

    // Free image memory
    for (unsigned int i = 0; i < NUMBER_OF_IMAGE_MEM; ++i)
    {
        is_FreeImageMem(m_hCam, m_imageMem[i], m_imageMemID[i]);
    }

    if (m_hCam != 0)
    {
        is_ExitCamera(m_hCam);
        m_hCam = 0;
    }
}

void CTimeLapseControl::Run()
{
    // Check successful initialization
    if (!m_initializationSuccessful)
    {
        printf("Initialization not complete, aborting image capturing!\n");
        return;
    }
    m_captureRunning = true;

    // Start capturing
    int ret = is_CaptureVideo(m_hCam, IS_DONT_WAIT);
    if (ret != IS_SUCCESS)
    {
        printf("Capture video error: %d\n", ret);
    }

    // Get the frametime for timeout calculation
    double frameRate_fps = 0.0;
    ret = is_SetFrameRate(m_hCam, IS_GET_FRAMERATE, &frameRate_fps);
    if (ret != IS_SUCCESS)
    {
        printf("Get framerate error: %d\n", ret);
    }
    unsigned int frameTime_ms = static_cast<unsigned int>(1000.0 / frameRate_fps);

    // Step into the capture loop
    char * currentImageData = 0;
    int currentImageID = 0;
    do
    {
        // Wait for the next image with doubled frametime as timeout
        ret = is_WaitForNextImage(m_hCam, 2 * frameTime_ms, &currentImageData, &currentImageID);
        if ((ret != IS_SUCCESS) && (ret != IS_CAPTURE_STATUS))
        {
            printf("Capture error: %d\n", ret);
        }

        // Check the current elapsed time if a new image should be stored to the disc (or for the first time)
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - m_lastTimeCaptured;
        if (!m_useNextImage || (static_cast<unsigned int>(elapsed_seconds.count()) > m_timeBetweenImages_s))
        {
            // Check the histogram for usability of the image
            if (!isHistogramOK(currentImageID))
            {
                m_useNextImage = false;
            }
            else
            {

                printf("Image captured! Next in %d seconds\n", m_timeBetweenImages_s);
                m_lastTimeCaptured = std::chrono::system_clock::now();
                m_useNextImage = true;

                // Build up timestamp string
                auto in_time_t = std::chrono::system_clock::to_time_t(m_lastTimeCaptured);
                std::stringstream ss;
#ifdef __linux__
                struct std::tm * timeInfo;
                timeInfo = localtime(&in_time_t);
                ss << m_path << "Image_" << std::put_time(timeInfo, "%Y-%m-%d_%H-%M-%S") << ".jpeg";
#else
                struct std::tm timeInfo;
                localtime_s(&timeInfo, &in_time_t);
                ss << m_path << "Image_" << std::put_time(&timeInfo, "%Y-%m-%d_%H-%M-%S") << ".jpeg";
#endif
                wchar_t fileName[1024];
                memset(fileName, 0, 1024);
                for (unsigned int i = 0; i < ss.str().length(); ++i)
                {
                    fileName[i] = ss.str().at(i);
                }

                // save the current image with a quality of 90%
                IMAGE_FILE_PARAMS ImageFileParams;
                unsigned int wrappedImageID = static_cast<unsigned int>(currentImageID);
                ImageFileParams.ppcImageMem = &currentImageData;
                ImageFileParams.pnImageID = &wrappedImageID;
                ImageFileParams.pwchFileName = fileName;
                ImageFileParams.nFileType = IS_IMG_JPG;
                ImageFileParams.nQuality = 90;
                ret = is_ImageFile(m_hCam, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams));
                if (ret != IS_SUCCESS)
                {
                    printf("Save image error: %d\n", ret);
                }
            }
        }

        // Unlock the used image buffer
        ret = is_UnlockSeqBuf(m_hCam, currentImageID, currentImageData);
        if (ret != IS_SUCCESS)
        {
            printf("Unlock buffer (1) error: %d\n", ret);
        }
    } while (m_captureRunning);

    // stop the capturing
    ret = is_StopLiveVideo(m_hCam, IS_FORCE_VIDEO_STOP);
    if (ret != IS_SUCCESS)
    {
        printf("Stop video error: %d\n", ret);
    }

    // Unlock all image memorys
    for (unsigned int i = 0; i < NUMBER_OF_IMAGE_MEM; ++i)
    {
        is_UnlockSeqBuf(m_hCam, currentImageID, currentImageData);
    }
}

void CTimeLapseControl::SetTimeBetweenCapturedImages(unsigned int timeBetweenCapturedImages_s)
{
    m_timeBetweenImages_s = timeBetweenCapturedImages_s;
}

void CTimeLapseControl::InitCamera()
{
    printf("Initializing the camera, please wait!\n");

    // Basic camera init for use
    int ret = is_InitCamera(&m_hCam, NULL);
    if (ret != IS_SUCCESS)
    {
        printf("Cant open camera, error: %d\n", ret);
        m_initializationSuccessful = false;
    }

    ret = is_ParameterSet(m_hCam, IS_PARAMETERSET_CMD_LOAD_EEPROM, NULL, 0);
    if (ret != IS_SUCCESS)
    {
        printf("Cant init settings, error: %d\n", ret);
        m_initializationSuccessful = false;
    }

    // Get the current color mode from the camera settings
    m_colorMode = is_SetColorMode(m_hCam, IS_GET_COLOR_MODE);

    // Get the current image size from the camera settings
    ret = is_AOI(m_hCam, IS_AOI_IMAGE_GET_AOI, (void*)&m_imageSize, sizeof(m_imageSize));
    if (ret != IS_SUCCESS)
    {
        printf("Cant get image size, error: %d\n", ret);
        m_initializationSuccessful = false;
    }

    printf("Camera successful initialized\n");
}

void CTimeLapseControl::Stop()
{
    m_captureRunning = false;
}

void CTimeLapseControl::SetImagePath(std::string path)
{
    m_path = path;
}

int getHistogramFaktorForColorMode(int colorMode)
{
    // For every color mode with mono (like) source format only one channel is used for the histogram
    switch (colorMode)
    {
    case IS_CM_MONO8:
    case IS_CM_MONO10:
    case IS_CM_MONO12:
    case IS_CM_MONO16:
    case IS_CM_SENSOR_RAW8:
    case IS_CM_SENSOR_RAW10:
    case IS_CM_SENSOR_RAW12:
    case IS_CM_SENSOR_RAW16:
        return 1;
        break;
    default:
        return 3;
    }
}

bool CTimeLapseControl::isHistogramOK(int imageID)
{
    // Alloc memory for the RGB-Histogram, set pointer for the different colors for easy access
    DWORD histogramBuffer [256*3];

    // Get the histogram for the given image ID
    int ret = is_GetImageHistogram(m_hCam, imageID, IS_CM_RGB8_PACKED, histogramBuffer);
    if (ret != IS_SUCCESS)
    {
        printf("Get image hostogram failed with error: %d\n", ret);
        return false;
    }

    // Build the average brightness
    unsigned int meanValue = 0;
    for (auto i = 1; i < 256 * getHistogramFaktorForColorMode(m_colorMode); ++i)
    {
        meanValue += histogramBuffer[i] * (i % 256);
    }
    meanValue /= m_imageSize.s32Width * m_imageSize.s32Height * getHistogramFaktorForColorMode(m_colorMode);

    // The brigtness mean value must be between an 150 for a usable image!
    // with less than 100 the image is way too dark, the target is 128 and a too bright image can be easily adopted
    // with a shorter exposure time so a brightness of over 150 is too bright.
    // Attention: the AES of the camera should be active to use this feature!
    return (meanValue > 100) && (meanValue < 150);
}

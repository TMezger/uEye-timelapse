#include "TimeLapseControl.h"
#include <stdio.h>
#include <sstream>
#include <iomanip>

CTimeLapseControl::CTimeLapseControl()
{
    // Init members
    m_hCam = 0;
    m_timeBetweenImages_s = 3600; // one hour as default
    m_captureRunning = false;
    m_lastTimeCaptured = std::chrono::system_clock::now();
    m_firstImageCaptured = false;
    m_initializationSuccessful = true;
    m_path = "BaucamImages\\";

    // Init camera
    InitCamera();

    // Init image memory for an image queue
    int ret = 0;
    for (unsigned int i = 0; i < NUMBER_OF_IMAGE_MEM; ++i)
    {
        int ret = is_AllocImageMem (m_hCam, 1280, 1024, 24, &m_imageMem[i], &m_imageMemID[i]);
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
    // exit image queue
    is_ExitImageQueue(m_hCam);

    // free image memory
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

    // start capturing
    int ret = is_CaptureVideo(m_hCam, IS_DONT_WAIT);
    if (ret != IS_SUCCESS)
    {
        printf("Capture video error: %d\n", ret);
    }

    // get the frametime for timeout calculation
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
        // wait for the next image with doubled frametime as timeout
        ret = is_WaitForNextImage(m_hCam, 2 * frameTime_ms, &currentImageData, &currentImageID);
        if ((ret != IS_SUCCESS) && (ret != IS_CAPTURE_STATUS))
        {
            printf("Capture error: %d\n", ret);
        }

        // check the current elapsed time if a new image should be stored to the disc (or for the first time)
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - m_lastTimeCaptured;
        if (!m_firstImageCaptured || (static_cast<unsigned int>(elapsed_seconds.count()) > m_timeBetweenImages_s))
        {
            printf("Image captured! Next in %d seconds\n", m_timeBetweenImages_s);
            m_lastTimeCaptured = std::chrono::system_clock::now();
            m_firstImageCaptured = true;

            // Build up timestamp string
            auto in_time_t = std::chrono::system_clock::to_time_t(m_lastTimeCaptured);
            std::stringstream ss;
            struct std::tm timeInfo;
            localtime_s(&timeInfo, &in_time_t);
            ss << m_path << "Image_" << std::put_time(&timeInfo, "%Y-%m-%d_%H-%M-%S") << ".jpeg";
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
        ret = is_UnlockSeqBuf(m_hCam, currentImageID, currentImageData);
        if (ret != IS_SUCCESS)
        {
            printf("Unlock buffer (2) error: %d\n", ret);
        }
    }
}

void CTimeLapseControl::SetTimeBetweenCapturedImages(unsigned int timeBetweenCapturedImages_s)
{
    m_timeBetweenImages_s = timeBetweenCapturedImages_s;
}

void CTimeLapseControl::InitCamera()
{
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


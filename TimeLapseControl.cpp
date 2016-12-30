#include "TimeLapseControl.h"
#include <stdio.h>

CTimeLapseControl::CTimeLapseControl()
{
    // Init members
    hCam = 0;
    m_timeBetweenImages_s = 3600; // one hour as default

    // Init camera
    InitCamera();

    // Init image memory
    //TODO

    // Init capture proccess
    //TODO
}

CTimeLapseControl::~CTimeLapseControl()
{
    if (hCam != 0)
    {
        is_ExitCamera(hCam);
        hCam = 0;
    }
}

void CTimeLapseControl::Run()
{
    // start capturing
    //TODO
}

void CTimeLapseControl::SetTimeBetweenCapturedImages(unsigned int timeBetweenCapturedImages_s)
{
    m_timeBetweenImages_s = timeBetweenCapturedImages_s;
}

void CTimeLapseControl::InitCamera()
{
    int ret = is_InitCamera(&hCam, NULL);
    if (ret != IS_SUCCESS)
    {
        printf("Cant open camera, error: %d", ret);
    }

    ret = is_ParameterSet(hCam, IS_PARAMETERSET_CMD_LOAD_EEPROM, NULL, 0);
    if (ret != IS_SUCCESS)
    {
        printf("Cant init settings, error: %d", ret);
    }

    printf("Camera successful initialized\n");
}

#include "TimeLapseControl.h"
#include <stdio.h>

CTimeLapseControl::CTimeLapseControl()
{
	m_timeBetweenImages_s = 3600; // one hour as default
}

CTimeLapseControl::~CTimeLapseControl()
{
}

void CTimeLapseControl::Run()
{
	InitCamera();
}

void CTimeLapseControl::SetTimeBetweenCapturedImages(unsigned int timeBetweenCapturedImages_s)
{
	m_timeBetweenImages_s = timeBetweenCapturedImages_s;
}

void CTimeLapseControl::InitCamera()
{
	printf("Camera successful initialized\n");
}
#include "TimeLapseControl.h"

int main()
{
    CTimeLapseControl myControl;
    myControl.SetTimeBetweenCapturedImages(5);
    myControl.SetImagePath(std::string("BaucamImages\\"));
    myControl.Run();
    return 0;
}
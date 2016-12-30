#include "TimeLapseControl.h"

int main()
{
    CTimeLapseControl myControl;
    myControl.SetTimeBetweenCapturedImages(3);
    myControl.Run();
    return 0;
}
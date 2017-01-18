#include "TimeLapseControl.h"
#include <thread>
#include <iostream>

void waitForStop(CTimeLapseControl * myControl)
{
    printf("Enter 'q' to stop the aquisition!\n");
    char c = 0;
    do
    {
        std::cin >> c;
    } while (c != 'q' && c!= 'Q');
    printf("Stopping, please wait a moment ..\n");
    myControl->Stop();
}

int main()
{
    CTimeLapseControl myControl;
    myControl.SetTimeBetweenCapturedImages(60);
    myControl.SetImagePath(std::string("BaucamImages/"));
    std::thread t1 (waitForStop, &myControl);
    myControl.Run();
    t1.join();
    return 0;
}
class CTimeLapseControl
{
public:
	CTimeLapseControl();
	~CTimeLapseControl();
	void Run();
	void SetTimeBetweenCapturedImages(unsigned int timeBetweenCapturedImages_s);

private:
	unsigned int m_timeBetweenImages_s;

	void InitCamera();
};
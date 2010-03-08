#ifndef ADLER_DASHBOARD
#define ADLER_DASHBOARD

#include "../includes.h"

class DashboardComponent : public Component, public Timer
{
	Label* playHeadLabel;
	Label* cpuUsageLabel;
public:
	DashboardComponent();

	void paint(Graphics& g);
	void resized();

	void timerCallback();
};

#endif

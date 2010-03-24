#ifndef ADLER_DASHBOARD
#define ADLER_DASHBOARD

#include "../includes.h"
#include "SettingsSnapshot.h"

class DashboardComponent : public Component, public Timer
{
	Label* playHeadLabel;
	Label* cpuUsageLabel;

	Array<SettingsSnapshotSlotComponent*> snapshotSlots;

public:
	DashboardComponent();
	~DashboardComponent();

	void paint(Graphics& g);
	void resized();

	void timerCallback();
};

#endif

#include "Dashboard.h"
#include "MainHostWindow.h"

DashboardComponent::DashboardComponent()
{
	
	addAndMakeVisible(playHeadLabel = new Label(T("playHeadLabel"), T("Playhead: ")));
	playHeadLabel->setColour(Label::textColourId, Colours::white);
	
	addAndMakeVisible(cpuUsageLabel = new Label(T("cpuUsageLabel"), T("CPU: ")));
	cpuUsageLabel->setColour(Label::textColourId, Colours::white);

	for (int i=0; i<4; ++i)
	{
		SettingsSnapshotSlotComponent* c = new SettingsSnapshotSlotComponent();		
		snapshotSlots.add(c);
		addAndMakeVisible(c);
	}

	startTimer(1000/45);
}

DashboardComponent::~DashboardComponent()
{
	deleteAllChildren();
}

void DashboardComponent::paint(Graphics &g)
{
	//g.fillAll(Colours::linen);
}

void DashboardComponent::resized()
{
	playHeadLabel->setBounds(16, 16, 256, 16);
	cpuUsageLabel->setBounds(16, 32, 256, 16);

	for (int i=0; i<snapshotSlots.size(); ++i)
	{
		snapshotSlots.getUnchecked(i)->setBounds(256+32 + i*(getHeight()-16), 16, getHeight()-32, getHeight()-32);
	}
}

void DashboardComponent::timerCallback()
{
	const AudioDeviceManager* manager = static_cast<MainHostWindow*>(getTopLevelComponent())->getAudioDeviceManager();
	cpuUsageLabel->setText(T("CPU: ") + String(manager->getCpuUsage(), 2) + T("%"), false);
	repaint();
}


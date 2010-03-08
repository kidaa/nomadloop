#include "Dashboard.h"
#include "MainHostWindow.h"

DashboardComponent::DashboardComponent()
{
	
	addAndMakeVisible(playHeadLabel = new Label(T("playHeadLabel"), T("Playhead: ")));
	playHeadLabel->setColour(Label::textColourId, Colours::white);
	
	addAndMakeVisible(cpuUsageLabel = new Label(T("cpuUsageLabel"), T("CPU: ")));
	cpuUsageLabel->setColour(Label::textColourId, Colours::white);

	startTimer(1000/45);
}

void DashboardComponent::paint(Graphics &g)
{
	//g.fillAll(Colours::linen);
}

void DashboardComponent::resized()
{
	playHeadLabel->setBounds(16, 16, 256, 16);
	cpuUsageLabel->setBounds(16, 32, 256, 16);
}

void DashboardComponent::timerCallback()
{
	const AudioDeviceManager* manager = static_cast<MainHostWindow*>(getTopLevelComponent())->getAudioDeviceManager();
	cpuUsageLabel->setText(T("CPU: ") + String(manager->getCpuUsage(), 2) + T("%"), false);
	repaint();
}


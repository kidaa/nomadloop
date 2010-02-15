#include "LoopControls.h"

LoopComponent::LoopComponent()
: loop(LoopManager::getInstance()->getMasterLoop())
{
	startTimer(1000/45);
}

void LoopComponent::paint(Graphics &g)
{
	g.setColour(Colours::green);
	g.drawRect(0, 0, getWidth(), getHeight());
	if (loop != 0)
	{
		AudioLoopProcessor* audioLoop = dynamic_cast<AudioLoopProcessor*>(loop);
		if (audioLoop != 0)
		{
		}

		g.setColour(Colours::green);
		g.fillRect(0, 0, loop->getScrubPositionInSamples()*getWidth()/loop->getLengthInSamples(), getHeight());
	}

}

void LoopComponent::timerCallback()
{
	repaint();
}

// =====================================

ArrangementComponent::ArrangementComponent()
{
}

void ArrangementComponent::paint(Graphics &g)
{
}

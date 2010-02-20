#include "LoopControls.h"
#include "MainHostWindow.h"

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
		if (loop->getLengthInSamples() > 0)
			g.fillRect(0, 0, loop->getScrubPositionInSamples()*getWidth()/loop->getLengthInSamples(), getHeight());

		MidiLoopProcessor* midiLoop = dynamic_cast<MidiLoopProcessor*>(loop);
		if (midiLoop != 0)
		{
			g.setColour(Colours::white);
			g.drawText(midiLoop->getEstimatedKey().getName(), 4, 4, getWidth()-8, 16, Justification::centredLeft, true);
		}
	}

}

void LoopComponent::timerCallback()
{
	repaint();
}

void LoopComponent::mouseDown(const MouseEvent& e)
{
	if (e.mods.isPopupMenu())
	{
		PopupMenu m;

		GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();

		for (int i=0; i<graphDoc->graph.getNumFilters(); ++i)
		{
			LoopProcessor* l = dynamic_cast<LoopProcessor*>(graphDoc->graph.getNode(i)->processor);
			if (l != 0)
			{
				m.addItem(i+1, l->getName());
			}
		}

		int choice = m.show() - 1;
		if (choice >= 0)
			this->loop = dynamic_cast<LoopProcessor*>(graphDoc->graph.getNode(choice)->processor);
	}
}

// =====================================

ArrangementComponent::ArrangementComponent()
{
}

void ArrangementComponent::paint(Graphics &g)
{
}

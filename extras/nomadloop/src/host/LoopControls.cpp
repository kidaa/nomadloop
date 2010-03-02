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

			g.setColour(Colours::lightcyan);
			midiLoop->drawMidiBuffer(g, getWidth(), getHeight());
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

		m.addItem(-1, T("Move"));
		m.addItem(-2, T("Resize"));
		m.addItem(-3, T("Delete"));
		m.addSeparator();
		
		GraphDocumentComponent* graphDoc = dynamic_cast<MainHostWindow*>(getTopLevelComponent())->getGraphEditor();

		for (int i=0; i<graphDoc->graph.getNumFilters(); ++i)
		{
			LoopProcessor* l = dynamic_cast<LoopProcessor*>(graphDoc->graph.getNode(i)->processor);
			if (l != 0)
			{
				m.addItem(i+1, l->getName());
			}
		}

		int choice = m.show();
		if (choice > 0)
			this->loop = dynamic_cast<LoopProcessor*>(graphDoc->graph.getNode(choice-1)->processor);
		else if (choice == -1)
		{
			static_cast<SubviewComponent*>(this->getParentComponent())->setMovingComponent(this);
		}
		else if (choice == -2)
		{
			static_cast<SubviewComponent*>(this->getParentComponent())->setResizingComponent(this);
		}
		else if (choice == -3)
		{
			delete this;
		}
	}
}

// =====================================

ArrangementComponent::ArrangementComponent()
{
}

void ArrangementComponent::paint(Graphics &g)
{
}

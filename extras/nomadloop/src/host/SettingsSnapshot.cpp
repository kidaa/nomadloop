#include "SettingsSnapshot.h"

SettingsSnapshotSlotComponent::SettingsSnapshotSlotComponent()
{
}

void SettingsSnapshotSlotComponent::paint(Graphics &g)
{
	g.setColour(Colours::aliceblue);
	g.drawRect(0, 0, getWidth(), getHeight());
}

// ======================================

SettingsSnapshotter::SettingsSnapshotter()
: drawable(0)
{
}

void SettingsSnapshotter::paint(Graphics& g)
{	
	if (drawable != 0)
	{
		//g.drawImage(drawable, 0, 0, getWidth(), getHeight(), 0, 0, drawable->get
		drawable->drawWithin(g, Rectangle<float>(getWidth(), getHeight()), RectanglePlacement::fillDestination, 1.0f);
	}
}


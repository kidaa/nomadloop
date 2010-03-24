#ifndef ADLER_SNAPSHOT
#define ADLER_SNAPSHOT

#include "../includes.h"

class SettingsSnapshotSlotComponent : public Component
{
public:
	SettingsSnapshotSlotComponent();

	void paint(Graphics &g);
};

// This is a special control similar to a button.
// When pressed, it grabs the settings for the components in the same Subview
// and stores them as a snapshot in the dashboard, which can later be restored.
class SettingsSnapshotter : public Component
{
	Drawable* drawable;
public:
	SettingsSnapshotter();

	void paint(Graphics &g);
};

#endif

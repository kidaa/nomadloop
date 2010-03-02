#ifndef ADLER_LOOPCONTROLS
#define ADLER_LOOPCONTROLS

#include "../includes.h"
#include "Looper.h"

class LoopComponent : public Component, public Timer
{
	LoopProcessor* loop;

public:
	LoopComponent();

	void paint(Graphics &g);
	void timerCallback();

	void mouseDown(const MouseEvent &e);
};

class ArrangementComponent : public Component
{
public:
	ArrangementComponent();

	void paint(Graphics &g);
};

#endif

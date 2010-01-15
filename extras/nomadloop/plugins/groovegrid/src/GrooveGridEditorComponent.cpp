/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "includes.h"
#include "GrooveGridEditorComponent.h"

class GrooveGridCell : public Component
{
	Colour colour;
	int instrumentNumber;
	int step;

public:
	GrooveGridCell(int instrumentNumber, int step)
		:	colour((step/4)%2==0?Colours::lightgrey:Colours::grey),
			instrumentNumber(instrumentNumber),
			step(step)
	{
	}

	void setColour(const Colour &c)
	{
		colour = c;
		repaint();
	}

	void paint(Graphics &g)
	{
		this->getLookAndFeel().drawGlassLozenge(g, 0.f, 0.f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), colour, 1.0f, 8.0f, false, false, false, false);
	}

	void mouseDown(const MouseEvent &m)
	{
		GrooveGridFilter* filter = static_cast<GrooveGridEditorComponent*>(getParentComponent())->getFilter();
		filter->grid[step][instrumentNumber] = !filter->grid[step][instrumentNumber];
		setColour(filter->grid[step][instrumentNumber]?Colours::orange:((step/4)%2==0?Colours::lightgrey:Colours::grey));
	}
};


//==============================================================================
// quick-and-dirty function to format a timecode string
static const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);
    const tchar* const sign = (seconds < 0) ? T("-") : T("");

    const int hours = (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;

    return String::formatted (T("%s%02d:%02d:%02d:%03d"),
                              sign, hours, mins, secs,
                              roundDoubleToInt (absSecs * 1000) % 1000);
}

// quick-and-dirty function to format a bars/beats string
static const String ppqToBarsBeatsString (const double ppq,
                                          const double lastBarPPQ,
                                          const int numerator,
                                          const int denominator)
{
    if (numerator == 0 || denominator == 0)
        return T("1|1|0");

    const int ppqPerBar = (numerator * 4 / denominator);
    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;

    const int bar       = ((int) ppq) / ppqPerBar + 1;
    const int beat      = ((int) beats) + 1;
    const int ticks     = ((int) (fmod (beats, 1.0) * 960.0));

    String s;
    s << bar << T('|') << beat << T('|') << ticks;
    return s;
}


//==============================================================================
GrooveGridEditorComponent::GrooveGridEditorComponent (GrooveGridFilter* const ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
    // create our gain slider..
    /*addAndMakeVisible (gainSlider = new Slider (T("gain")));
    gainSlider->addListener (this);
    gainSlider->setRange (0.0, 1.0, 0.01);
    gainSlider->setTooltip (T("changes the volume of the audio that runs through the plugin.."));

    // get the gain parameter from the filter and use it to set up our slider
    gainSlider->setValue (ownerFilter->getParameter (0), false);*/

    // add a label that will display the current timecode and status..
    addAndMakeVisible (infoLabel = new Label (String::empty, String::empty));

    // add the triangular resizer component for the bottom-right of the UI
    /*addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
    resizeLimits.setSizeLimits (800, 400, 800, 400);*/

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (800, 400);

	for (int i=0; i<16; ++i)
	for (int j=0; j<8; ++j)
	{
		GrooveGridCell* ggc = new GrooveGridCell(j, i);
		ggc->setBounds(i*48+8, j*48+8, 40, 40);
		//ggc->setColour((i/4)%2==0?Colours::lightgrey:Colours::grey);
		ggc->setColour(ownerFilter->grid[i][j]?Colours::orange:((i/4)%2==0?Colours::lightgrey:Colours::grey));
		addAndMakeVisible(ggc);
	}

    // register ourselves with the filter - it will use its ChangeBroadcaster base
    // class to tell us when something has changed, and this will call our changeListenerCallback()
    // method.
    ownerFilter->addChangeListener (this);
}

GrooveGridEditorComponent::~GrooveGridEditorComponent()
{
    getFilter()->removeChangeListener (this);

    deleteAllChildren();
}

//==============================================================================
void GrooveGridEditorComponent::paint (Graphics& g)
{
    // just clear the window
    g.fillAll (Colour::greyLevel (0.9f));
}

void GrooveGridEditorComponent::resized()
{
    //gainSlider->setBounds (10, 10, 200, 22);
    infoLabel->setBounds (10, 35, 450, 20);

    const int keyboardHeight = 70;
    //midiKeyboard->setBounds (4, getHeight() - keyboardHeight - 4,
    //                         getWidth() - 8, keyboardHeight);

    //resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);

    // if we've been resized, tell the filter so that it can store the new size
    // in its settings
    //getFilter()->lastUIWidth = getWidth();
    //getFilter()->lastUIHeight = getHeight();
}

//==============================================================================
void GrooveGridEditorComponent::changeListenerCallback (void* source)
{
    // this is the filter telling us that it's changed, so we'll update our
    // display of the time, midi message, etc.
    updateParametersFromFilter();
}

void GrooveGridEditorComponent::sliderValueChanged (Slider*)
{
    getFilter()->setParameterNotifyingHost (0, (float) gainSlider->getValue());
}

//==============================================================================
void GrooveGridEditorComponent::updateParametersFromFilter()
{
    GrooveGridFilter* const filter = getFilter();

    // we use this lock to make sure the processBlock() method isn't writing to the
    // lastMidiMessage variable while we're trying to read it, but be extra-careful to
    // only hold the lock for a minimum amount of time..
    filter->getCallbackLock().enter();

    // take a local copy of the info we need while we've got the lock..
    const AudioPlayHead::CurrentPositionInfo positionInfo (filter->lastPosInfo);
    const float newGain = filter->getParameter (0);

    // ..release the lock ASAP
    filter->getCallbackLock().exit();


    // ..and after releasing the lock, we're free to do the time-consuming UI stuff..
    String infoText;
    infoText << String (positionInfo.bpm, 2) << T(" bpm, ")
             << positionInfo.timeSigNumerator << T("/") << positionInfo.timeSigDenominator
             << T("  -  ") << timeToTimecodeString (positionInfo.timeInSeconds)
             << T("  -  ") << ppqToBarsBeatsString (positionInfo.ppqPosition,
                                                    positionInfo.ppqPositionOfLastBarStart,
                                                    positionInfo.timeSigNumerator,
                                                    positionInfo.timeSigDenominator);

    if (positionInfo.isPlaying)
        infoText << T("  (playing)");

    infoLabel->setText (infoText, false);

    /* Update our slider.

       (note that it's important here to tell the slider not to send a change
       message, because that would cause it to call the filter with a parameter
       change message again, and the values would drift out.
    */
    //gainSlider->setValue (newGain, false);

    /*setSize (filter->lastUIWidth,
             filter->lastUIHeight);*/
}

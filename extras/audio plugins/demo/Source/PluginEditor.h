/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINEDITOR_H_4ACCBAA__
#define __PLUGINEDITOR_H_4ACCBAA__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "PluginProcessor.h"


//==============================================================================
/** This is the editor component that our filter will display.
*/
class JuceDemoPluginAudioProcessorEditor  : public AudioProcessorEditor,
                                            public Slider::Listener,
                                            public Timer
{
public:
    JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor* ownerFilter);
    ~JuceDemoPluginAudioProcessorEditor();

    //==============================================================================
    void timerCallback();
    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider*);

private:
    MidiKeyboardComponent* midiKeyboard;
    Label* infoLabel;
    Slider* gainSlider;
    Slider* delaySlider;
    ResizableCornerComponent* resizer;
    ComponentBoundsConstrainer resizeLimits;

    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;

    JuceDemoPluginAudioProcessor* getProcessor() const
    {
        return static_cast <JuceDemoPluginAudioProcessor*> (getAudioProcessor());
    }

    void displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos);
};


#endif  // __PLUGINEDITOR_H_4ACCBAA__

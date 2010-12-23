#ifndef ADLER_CHORDSETTER
#define ADLER_CHORDSETTER

#include "../includes.h"

class ChordSetter : public AudioPluginInstance
{
public:
	Array<int> activeInputNotes;

public:
	ChordSetter();
	
	void fillInPluginDescription(PluginDescription &desc) const;

	const String getName() const;
	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void processBlock(AudioSampleBuffer &, MidiBuffer &);
	const String getInputChannelName(const int) const;
	const String getOutputChannelName(const int) const;	
	bool isInputChannelStereoPair(int) const;
	bool isOutputChannelStereoPair(int) const;
	bool acceptsMidi() const;
	bool producesMidi() const;
	bool hasEditor() const;
	AudioProcessorEditor* createEditor();
	int getNumParameters();
	const String getParameterName(int);
	float getParameter(int);
	const String getParameterText(int);
	void setParameter(int, float);
	int getNumPrograms();
	int getCurrentProgram();
	void setCurrentProgram(int);
	const String getProgramName(int);
	void changeProgramName(int, const String&);
	void getStateInformation(MemoryBlock&);
	void setStateInformation(const void *, int);
};

class ChordSetterEditor : public AudioProcessorEditor, public Timer,
                              public ChangeListener,
                              public SliderListener
{
public:
    /** Constructor.

        When created, this will register itself with the filter for changes. It's
        safe to assume that the filter won't be deleted before this object is.
    */
    ChordSetterEditor (ChordSetter* const owner);

    /** Destructor. */
    ~ChordSetterEditor();

    //==============================================================================
    /** Our demo filter is a ChangeBroadcaster, and will call us back when one of
        its parameters changes.
    */
    void changeListenerCallback (ChangeBroadcaster* source);

    void sliderValueChanged (Slider*);

	void timerCallback();

    //==============================================================================
    /** Standard Juce paint callback. */
    void paint (Graphics& g);

    /** Standard Juce resize callback. */
    void resized();

	
    // handy wrapper method to avoid having to cast the filter to a GrooveGridFilter
    // every time we need it..
    ChordSetter* getFilter() const throw()       { return (ChordSetter*) getAudioProcessor(); }

private:
    //==============================================================================    
    //MidiKeyboardComponent* midiKeyboard;
    Label* infoLabel;	
    TooltipWindow tooltipWindow;

    void updateParametersFromFilter();

};

#endif

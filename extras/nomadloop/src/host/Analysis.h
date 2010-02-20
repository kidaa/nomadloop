#ifndef ADLER_ANALYSIS
#define ADLER_ANALYSIS

#include "../includes.h"

class Key
{
public:
	enum Mode 
	{
		Unknown,
		Major,
		Minor
	};

private:
	Mode mode;
	int rootPitchClass;

public:
	Key(int pitchClass = 0, Mode mode = Unknown)
		: rootPitchClass(pitchClass), mode(mode)
	{
	}

	inline bool isMajor() const	{ return mode == Major; }
	inline bool isMinor() const { return mode == Minor; }
	inline Mode getMode() const { return mode; }
	inline int getRootPitchClass() const { return rootPitchClass; }
	String getName() const;
};

class KeyAnalyzer
{
	Key bestGuess;
	Key secondGuess;
public:
	const Key& getKey() const;

	void analyze(const MidiBuffer& buffer);	
};

#endif

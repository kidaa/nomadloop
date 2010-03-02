#include "Analysis.h"

String Key::getName() const
{
	String note;
	switch (rootPitchClass)
	{
		case 0: note = "C"; break;
		case 1: note = "C#"; break;
		case 2: note = "D"; break;
		case 3: note = "D#"; break;
		case 4: note = "E"; break;
		case 5: note = "F"; break;
		case 6: note = "F#"; break;
		case 7: note = "G"; break;
		case 8: note = "G#"; break;
		case 9: note = "A"; break;
		case 10: note = "A#"; break;
		case 11: note = "B"; break;
	}

	return note + String(isMajor()?T("maj"):isMinor()?T("min"):T("?"));
}

const Key& KeyAnalyzer::getKey() const
{
	return bestGuess;
}

const double majorKey[12] = {6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
const double minorKey[12] = {6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17};

// analyze using Krumhansl-Schmuckler algorithm
void KeyAnalyzer::analyze(const MidiBuffer& buffer)
{
	// TODO: analyze buffer to produce discrete pitches, i.e., eliminate bends

	// first, iterate through the buffer and see how many times the notes occur
	double total = 0;
	int occurrences[12] = {0};
	MidiBuffer::Iterator midiItor(buffer);
	MidiMessage message(0x80, 60, 0);
	int samplePos;
	
	while (midiItor.getNextEvent(message, samplePos))
	{
		if (message.isNoteOn())
		{
			occurrences[message.getNoteNumber()%12]++;			
		}
	}
	for (int i=0; i<12; ++i)
		total += occurrences[i];

	if (total <= 0)
		return;

	// determine mean frequency for pitch distribution
	double mean = total / 12.0;
	double majorMean = 3.4825;
	double minorMean = 3.709167;

	double ratingMajor[12];
	double ratingMinor[12];
	double majNumerator, majDenominator, majTemp;
	double minNumerator, minDenominator, minTemp;
	double temp;
	int subscript;

	for (int i=0; i<12; ++i)
	{
		majNumerator = minNumerator = 0;
		majDenominator = minDenominator = 0;
		majTemp = minTemp = temp = 0;

		for (int j=0; j<12; ++j)
		{
			subscript = (i+j) % 12;
			temp += (occurrences[subscript] - mean) * (occurrences[subscript] - mean);

			// major keys
			majNumerator += (majorKey[j]-majorMean) * (occurrences[subscript] - mean);
			majTemp += (majorKey[j]-majorMean) * (majorKey[j]-majorMean);

			minNumerator += (minorKey[j]-minorMean) * (occurrences[subscript] - mean);
			minTemp += (minorKey[j]-minorMean) * (minorKey[j]-minorMean);
		}

		majDenominator = sqrt(majTemp * temp);
		minDenominator = sqrt(minTemp * temp);

		if (majDenominator == 0 || minDenominator == 0)
			return;

		ratingMajor[i] = majNumerator / majDenominator;
		ratingMinor[i] = minNumerator / minDenominator;
	}

	// find the highest ranked key
	double bestKey = 0.0;
	Key::Mode mode = Key::Unknown;
	int pitchClass = 0;

	for (int i=0; i<12; ++i)
	{
		if (ratingMajor[i] > bestKey)
		{
			bestKey = ratingMajor[i];
			mode = Key::Major;
			pitchClass = i;
		}
		if (ratingMinor[i] > bestKey)
		{
			bestKey = ratingMinor[i];
			mode = Key::Minor;
			pitchClass = i;
		}
	}
	bestGuess = Key(pitchClass, mode);

	// find the second highest ranked key
	double secondBestKey = 0;
	Key::Mode secondMode = Key::Unknown;
	int secondPitchClass = 0;
	for (int i=0; i<12; ++i)
	{
		if (ratingMajor[i] != bestKey && ratingMajor[i] > secondBestKey)
		{
			secondBestKey = ratingMajor[i];
			secondMode = Key::Major;
			secondPitchClass = i;
		}
		if (ratingMinor[i] != bestKey && ratingMinor[i] > secondBestKey)
		{
			secondBestKey = ratingMinor[i];
			secondMode = Key::Minor;
			secondPitchClass = i;
		}
	}
}


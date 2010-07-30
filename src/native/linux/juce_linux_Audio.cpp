/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_ALSA

//==============================================================================
static const int maxNumChans = 64;


//==============================================================================
static void getDeviceSampleRates (snd_pcm_t* handle, Array <int>& rates)
{
    const int ratesToTry[] = { 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 0 };

    snd_pcm_hw_params_t* hwParams;
    snd_pcm_hw_params_alloca (&hwParams);

    for (int i = 0; ratesToTry[i] != 0; ++i)
    {
        if (snd_pcm_hw_params_any (handle, hwParams) >= 0
             && snd_pcm_hw_params_test_rate (handle, hwParams, ratesToTry[i], 0) == 0)
        {
            rates.addIfNotAlreadyThere (ratesToTry[i]);
        }
    }
}

static void getDeviceNumChannels (snd_pcm_t* handle, unsigned int* minChans, unsigned int* maxChans)
{
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca (&params);

    if (snd_pcm_hw_params_any (handle, params) >= 0)
    {
        snd_pcm_hw_params_get_channels_min (params, minChans);
        snd_pcm_hw_params_get_channels_max (params, maxChans);
    }
}

static void getDeviceProperties (const String& deviceID,
                                 unsigned int& minChansOut,
                                 unsigned int& maxChansOut,
                                 unsigned int& minChansIn,
                                 unsigned int& maxChansIn,
                                 Array <int>& rates)
{
    if (deviceID.isEmpty())
        return;

    snd_ctl_t* handle;

    if (snd_ctl_open (&handle, deviceID.upToLastOccurrenceOf (",", false, false).toUTF8(), SND_CTL_NONBLOCK) >= 0)
    {
        snd_pcm_info_t* info;
        snd_pcm_info_alloca (&info);

        snd_pcm_info_set_stream (info, SND_PCM_STREAM_PLAYBACK);
        snd_pcm_info_set_device (info, deviceID.fromLastOccurrenceOf (",", false, false).getIntValue());
        snd_pcm_info_set_subdevice (info, 0);

        if (snd_ctl_pcm_info (handle, info) >= 0)
        {
            snd_pcm_t* pcmHandle;
            if (snd_pcm_open (&pcmHandle, deviceID.toUTF8(), SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC | SND_PCM_NONBLOCK ) >= 0)
            {
                getDeviceNumChannels (pcmHandle, &minChansOut, &maxChansOut);
                getDeviceSampleRates (pcmHandle, rates);

                snd_pcm_close (pcmHandle);
            }
        }

        snd_pcm_info_set_stream (info, SND_PCM_STREAM_CAPTURE);

        if (snd_ctl_pcm_info (handle, info) >= 0)
        {
            snd_pcm_t* pcmHandle;
            if (snd_pcm_open (&pcmHandle, deviceID.toUTF8(), SND_PCM_STREAM_CAPTURE, SND_PCM_ASYNC | SND_PCM_NONBLOCK ) >= 0)
            {
                getDeviceNumChannels (pcmHandle, &minChansIn, &maxChansIn);

                if (rates.size() == 0)
                    getDeviceSampleRates (pcmHandle, rates);

                snd_pcm_close (pcmHandle);
            }
        }

        snd_ctl_close (handle);
    }
}

//==============================================================================
class ALSADevice
{
public:
    ALSADevice (const String& deviceID,
                const bool forInput)
        : handle (0),
          bitDepth (16),
          numChannelsRunning (0),
          isInput (forInput),
          sampleFormat (AudioDataConverters::int16LE)
    {
        failed (snd_pcm_open (&handle, deviceID.toUTF8(),
                              forInput ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK,
                              SND_PCM_ASYNC));
    }

    ~ALSADevice()
    {
        if (handle != 0)
            snd_pcm_close (handle);
    }

    bool setParameters (unsigned int sampleRate, int numChannels, int bufferSize)
    {
        if (handle == 0)
            return false;

        snd_pcm_hw_params_t* hwParams;
        snd_pcm_hw_params_alloca (&hwParams);

        if (failed (snd_pcm_hw_params_any (handle, hwParams)))
            return false;

        if (snd_pcm_hw_params_set_access (handle, hwParams, SND_PCM_ACCESS_RW_NONINTERLEAVED) >= 0)
            isInterleaved = false;
        else if (snd_pcm_hw_params_set_access (handle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED) >= 0)
            isInterleaved = true;
        else
        {
            jassertfalse;
            return false;
        }

        const int formatsToTry[] = { SND_PCM_FORMAT_FLOAT_LE, 32, AudioDataConverters::float32LE,
                                     SND_PCM_FORMAT_FLOAT_BE, 32, AudioDataConverters::float32BE,
                                     SND_PCM_FORMAT_S32_LE, 32, AudioDataConverters::int32LE,
                                     SND_PCM_FORMAT_S32_BE, 32, AudioDataConverters::int32BE,
                                     SND_PCM_FORMAT_S24_3LE, 24, AudioDataConverters::int24LE,
                                     SND_PCM_FORMAT_S24_3BE, 24, AudioDataConverters::int24BE,
                                     SND_PCM_FORMAT_S16_LE, 16, AudioDataConverters::int16LE,
                                     SND_PCM_FORMAT_S16_BE, 16, AudioDataConverters::int16BE };
        bitDepth = 0;

        for (int i = 0; i < numElementsInArray (formatsToTry); i += 3)
        {
            if (snd_pcm_hw_params_set_format (handle, hwParams, (_snd_pcm_format) formatsToTry [i]) >= 0)
            {
                bitDepth = formatsToTry [i + 1];
                sampleFormat = (AudioDataConverters::DataFormat) formatsToTry [i + 2];
                break;
            }
        }

        if (bitDepth == 0)
        {
            error = "device doesn't support a compatible PCM format";
            DBG ("ALSA error: " + error + "\n");
            return false;
        }

        int dir = 0;
        unsigned int periods = 4;
        snd_pcm_uframes_t samplesPerPeriod = bufferSize;

        if (failed (snd_pcm_hw_params_set_rate_near (handle, hwParams, &sampleRate, 0))
            || failed (snd_pcm_hw_params_set_channels (handle, hwParams, numChannels))
            || failed (snd_pcm_hw_params_set_periods_near (handle, hwParams, &periods, &dir))
            || failed (snd_pcm_hw_params_set_period_size_near (handle, hwParams, &samplesPerPeriod, &dir))
            || failed (snd_pcm_hw_params (handle, hwParams)))
        {
            return false;
        }

        snd_pcm_sw_params_t* swParams;
        snd_pcm_sw_params_alloca (&swParams);
        snd_pcm_uframes_t boundary;

        if (failed (snd_pcm_sw_params_current (handle, swParams))
            || failed (snd_pcm_sw_params_get_boundary (swParams, &boundary))
            || failed (snd_pcm_sw_params_set_silence_threshold (handle, swParams, 0))
            || failed (snd_pcm_sw_params_set_silence_size (handle, swParams, boundary))
            || failed (snd_pcm_sw_params_set_start_threshold (handle, swParams, samplesPerPeriod))
            || failed (snd_pcm_sw_params_set_stop_threshold (handle, swParams, boundary))
            || failed (snd_pcm_sw_params (handle, swParams)))
        {
            return false;
        }

        /*
#if JUCE_DEBUG
        // enable this to dump the config of the devices that get opened
        snd_output_t* out;
        snd_output_stdio_attach (&out, stderr, 0);
        snd_pcm_hw_params_dump (hwParams, out);
        snd_pcm_sw_params_dump (swParams, out);
#endif
        */

        numChannelsRunning = numChannels;

        return true;
    }

    //==============================================================================
    bool write (AudioSampleBuffer& outputChannelBuffer, const int numSamples)
    {
        jassert (numChannelsRunning <= outputChannelBuffer.getNumChannels());
        float** const data = outputChannelBuffer.getArrayOfChannels();

        if (isInterleaved)
        {
            scratch.ensureSize (sizeof (float) * numSamples * numChannelsRunning, false);
            float* interleaved = static_cast <float*> (scratch.getData());

            AudioDataConverters::interleaveSamples ((const float**) data, interleaved, numSamples, numChannelsRunning);
            AudioDataConverters::convertFloatToFormat (sampleFormat, interleaved, interleaved, numSamples * numChannelsRunning);

            snd_pcm_sframes_t num = snd_pcm_writei (handle, interleaved, numSamples);

            if (failed (num) && num != -EPIPE && num != -ESTRPIPE)
                return false;
        }
        else
        {
            for (int i = 0; i < numChannelsRunning; ++i)
                if (data[i] != 0)
                    AudioDataConverters::convertFloatToFormat (sampleFormat, data[i], data[i], numSamples);

            snd_pcm_sframes_t num = snd_pcm_writen (handle, (void**) data, numSamples);

            if (failed (num))
            {
                if (num == -EPIPE)
                {
                    if (failed (snd_pcm_prepare (handle)))
                        return false;
                }
                else if (num != -ESTRPIPE)
                    return false;
            }
        }

        return true;
    }

    bool read (AudioSampleBuffer& inputChannelBuffer, const int numSamples)
    {
        jassert (numChannelsRunning <= inputChannelBuffer.getNumChannels());
        float** const data = inputChannelBuffer.getArrayOfChannels();

        if (isInterleaved)
        {
            scratch.ensureSize (sizeof (float) * numSamples * numChannelsRunning, false);
            float* interleaved = static_cast <float*> (scratch.getData());

            snd_pcm_sframes_t num = snd_pcm_readi (handle, interleaved, numSamples);

            if (failed (num))
            {
                if (num == -EPIPE)
                {
                    if (failed (snd_pcm_prepare (handle)))
                        return false;
                }
                else if (num != -ESTRPIPE)
                    return false;
            }

            AudioDataConverters::convertFormatToFloat (sampleFormat, interleaved, interleaved, numSamples * numChannelsRunning);
            AudioDataConverters::deinterleaveSamples (interleaved, data, numSamples, numChannelsRunning);
        }
        else
        {
            snd_pcm_sframes_t num = snd_pcm_readn (handle, (void**) data, numSamples);

            if (failed (num) && num != -EPIPE && num != -ESTRPIPE)
                return false;

            for (int i = 0; i < numChannelsRunning; ++i)
                if (data[i] != 0)
                    AudioDataConverters::convertFormatToFloat (sampleFormat, data[i], data[i], numSamples);
        }

        return true;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    snd_pcm_t* handle;
    String error;
    int bitDepth, numChannelsRunning;

    //==============================================================================
private:
    const bool isInput;
    bool isInterleaved;
    MemoryBlock scratch;
    AudioDataConverters::DataFormat sampleFormat;

    //==============================================================================
    bool failed (const int errorNum)
    {
        if (errorNum >= 0)
            return false;

        error = snd_strerror (errorNum);
        DBG ("ALSA error: " + error + "\n");
        return true;
    }
};

//==============================================================================
class ALSAThread  : public Thread
{
public:
    ALSAThread (const String& inputId_,
                const String& outputId_)
        : Thread ("Juce ALSA"),
          sampleRate (0),
          bufferSize (0),
          callback (0),
          inputId (inputId_),
          outputId (outputId_),
          numCallbacks (0),
          inputChannelBuffer (1, 1),
          outputChannelBuffer (1, 1)
    {
        initialiseRatesAndChannels();
    }

    ~ALSAThread()
    {
        close();
    }

    void open (BigInteger inputChannels,
               BigInteger outputChannels,
               const double sampleRate_,
               const int bufferSize_)
    {
        close();

        error = String::empty;
        sampleRate = sampleRate_;
        bufferSize = bufferSize_;

        inputChannelBuffer.setSize (jmax ((int) minChansIn, inputChannels.getHighestBit()) + 1, bufferSize);
        inputChannelBuffer.clear();
        inputChannelDataForCallback.clear();
        currentInputChans.clear();

        if (inputChannels.getHighestBit() >= 0)
        {
            for (int i = 0; i <= jmax (inputChannels.getHighestBit(), (int) minChansIn); ++i)
            {
                if (inputChannels[i])
                {
                    inputChannelDataForCallback.add (inputChannelBuffer.getSampleData (i));
                    currentInputChans.setBit (i);
                }
            }
        }

        outputChannelBuffer.setSize (jmax ((int) minChansOut, outputChannels.getHighestBit()) + 1, bufferSize);
        outputChannelBuffer.clear();
        outputChannelDataForCallback.clear();
        currentOutputChans.clear();

        if (outputChannels.getHighestBit() >= 0)
        {
            for (int i = 0; i <= jmax (outputChannels.getHighestBit(), (int) minChansOut); ++i)
            {
                if (outputChannels[i])
                {
                    outputChannelDataForCallback.add (outputChannelBuffer.getSampleData (i));
                    currentOutputChans.setBit (i);
                }
            }
        }

        if (outputChannelDataForCallback.size() > 0 && outputId.isNotEmpty())
        {
            outputDevice = new ALSADevice (outputId, false);

            if (outputDevice->error.isNotEmpty())
            {
                error = outputDevice->error;
                outputDevice = 0;
                return;
            }

            currentOutputChans.setRange (0, minChansOut, true);

            if (! outputDevice->setParameters ((unsigned int) sampleRate,
                                               jlimit ((int) minChansOut, (int) maxChansOut, currentOutputChans.getHighestBit() + 1),
                                               bufferSize))
            {
                error = outputDevice->error;
                outputDevice = 0;
                return;
            }
        }

        if (inputChannelDataForCallback.size() > 0 && inputId.isNotEmpty())
        {
            inputDevice = new ALSADevice (inputId, true);

            if (inputDevice->error.isNotEmpty())
            {
                error = inputDevice->error;
                inputDevice = 0;
                return;
            }

            currentInputChans.setRange (0, minChansIn, true);

            if (! inputDevice->setParameters ((unsigned int) sampleRate,
                                              jlimit ((int) minChansIn, (int) maxChansIn, currentInputChans.getHighestBit() + 1),
                                              bufferSize))
            {
                error = inputDevice->error;
                inputDevice = 0;
                return;
            }
        }

        if (outputDevice == 0 && inputDevice == 0)
        {
            error = "no channels";
            return;
        }

        if (outputDevice != 0 && inputDevice != 0)
        {
            snd_pcm_link (outputDevice->handle, inputDevice->handle);
        }

        if (inputDevice != 0 && failed (snd_pcm_prepare (inputDevice->handle)))
            return;

        if (outputDevice != 0 && failed (snd_pcm_prepare (outputDevice->handle)))
            return;

        startThread (9);

        int count = 1000;

        while (numCallbacks == 0)
        {
            sleep (5);

            if (--count < 0 || ! isThreadRunning())
            {
                error = "device didn't start";
                break;
            }
        }
    }

    void close()
    {
        stopThread (6000);

        inputDevice = 0;
        outputDevice = 0;

        inputChannelBuffer.setSize (1, 1);
        outputChannelBuffer.setSize (1, 1);

        numCallbacks = 0;
    }

    void setCallback (AudioIODeviceCallback* const newCallback) throw()
    {
        const ScopedLock sl (callbackLock);
        callback = newCallback;
    }

    void run()
    {
        while (! threadShouldExit())
        {
            if (inputDevice != 0)
            {
                if (! inputDevice->read (inputChannelBuffer, bufferSize))
                {
                    DBG ("ALSA: read failure");
                    break;
                }
            }

            if (threadShouldExit())
                break;

            {
                const ScopedLock sl (callbackLock);
                ++numCallbacks;

                if (callback != 0)
                {
                    callback->audioDeviceIOCallback ((const float**) inputChannelDataForCallback.getRawDataPointer(),
                                                     inputChannelDataForCallback.size(),
                                                     outputChannelDataForCallback.getRawDataPointer(),
                                                     outputChannelDataForCallback.size(),
                                                     bufferSize);
                }
                else
                {
                    for (int i = 0; i < outputChannelDataForCallback.size(); ++i)
                        zeromem (outputChannelDataForCallback[i], sizeof (float) * bufferSize);
                }
            }

            if (outputDevice != 0)
            {
                failed (snd_pcm_wait (outputDevice->handle, 2000));

                if (threadShouldExit())
                    break;

                failed (snd_pcm_avail_update (outputDevice->handle));

                if (! outputDevice->write (outputChannelBuffer, bufferSize))
                {
                    DBG ("ALSA: write failure");
                    break;
                }
            }
        }
    }

    int getBitDepth() const throw()
    {
        if (outputDevice != 0)
            return outputDevice->bitDepth;

        if (inputDevice != 0)
            return inputDevice->bitDepth;

        return 16;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    String error;
    double sampleRate;
    int bufferSize;
    BigInteger currentInputChans, currentOutputChans;

    Array <int> sampleRates;
    StringArray channelNamesOut, channelNamesIn;
    AudioIODeviceCallback* callback;

private:
    //==============================================================================
    const String inputId, outputId;
    ScopedPointer<ALSADevice> outputDevice, inputDevice;
    int numCallbacks;

    CriticalSection callbackLock;

    AudioSampleBuffer inputChannelBuffer, outputChannelBuffer;
    Array<float*> inputChannelDataForCallback, outputChannelDataForCallback;

    unsigned int minChansOut, maxChansOut;
    unsigned int minChansIn, maxChansIn;

    bool failed (const int errorNum)
    {
        if (errorNum >= 0)
            return false;

        error = snd_strerror (errorNum);
        DBG ("ALSA error: " + error + "\n");
        return true;
    }

    void initialiseRatesAndChannels()
    {
        sampleRates.clear();
        channelNamesOut.clear();
        channelNamesIn.clear();
        minChansOut = 0;
        maxChansOut = 0;
        minChansIn = 0;
        maxChansIn = 0;
        unsigned int dummy = 0;

        getDeviceProperties (inputId, dummy, dummy, minChansIn, maxChansIn, sampleRates);
        getDeviceProperties (outputId, minChansOut, maxChansOut, dummy, dummy, sampleRates);

        unsigned int i;
        for (i = 0; i < maxChansOut; ++i)
            channelNamesOut.add ("channel " + String ((int) i + 1));

        for (i = 0; i < maxChansIn; ++i)
            channelNamesIn.add ("channel " + String ((int) i + 1));
    }
};


//==============================================================================
class ALSAAudioIODevice   : public AudioIODevice
{
public:
    ALSAAudioIODevice (const String& deviceName,
                       const String& inputId_,
                       const String& outputId_)
        : AudioIODevice (deviceName, "ALSA"),
          inputId (inputId_),
          outputId (outputId_),
          isOpen_ (false),
          isStarted (false),
          internal (inputId_, outputId_)
    {
    }

    ~ALSAAudioIODevice()
    {
    }

    const StringArray getOutputChannelNames()
    {
        return internal.channelNamesOut;
    }

    const StringArray getInputChannelNames()
    {
        return internal.channelNamesIn;
    }

    int getNumSampleRates()
    {
        return internal.sampleRates.size();
    }

    double getSampleRate (int index)
    {
        return internal.sampleRates [index];
    }

    int getNumBufferSizesAvailable()
    {
        return 50;
    }

    int getBufferSizeSamples (int index)
    {
        int n = 16;
        for (int i = 0; i < index; ++i)
            n += n < 64 ? 16
                        : (n < 512 ? 32
                                   : (n < 1024 ? 64
                                               : (n < 2048 ? 128 : 256)));

        return n;
    }

    int getDefaultBufferSize()
    {
        return 512;
    }

    const String open (const BigInteger& inputChannels,
                       const BigInteger& outputChannels,
                       double sampleRate,
                       int bufferSizeSamples)
    {
        close();

        if (bufferSizeSamples <= 0)
            bufferSizeSamples = getDefaultBufferSize();

        if (sampleRate <= 0)
        {
            for (int i = 0; i < getNumSampleRates(); ++i)
            {
                if (getSampleRate (i) >= 44100)
                {
                    sampleRate = getSampleRate (i);
                    break;
                }
            }
        }

        internal.open (inputChannels, outputChannels,
                       sampleRate, bufferSizeSamples);

        isOpen_ = internal.error.isEmpty();
        return internal.error;
    }

    void close()
    {
        stop();
        internal.close();
        isOpen_ = false;
    }

    bool isOpen()
    {
        return isOpen_;
    }

    int getCurrentBufferSizeSamples()
    {
        return internal.bufferSize;
    }

    double getCurrentSampleRate()
    {
        return internal.sampleRate;
    }

    int getCurrentBitDepth()
    {
        return internal.getBitDepth();
    }

    const BigInteger getActiveOutputChannels() const
    {
        return internal.currentOutputChans;
    }

    const BigInteger getActiveInputChannels() const
    {
        return internal.currentInputChans;
    }

    int getOutputLatencyInSamples()
    {
        return 0;
    }

    int getInputLatencyInSamples()
    {
        return 0;
    }

    void start (AudioIODeviceCallback* callback)
    {
        if (! isOpen_)
            callback = 0;

        if (callback != 0)
            callback->audioDeviceAboutToStart (this);

        internal.setCallback (callback);

        isStarted = (callback != 0);
    }

    void stop()
    {
        AudioIODeviceCallback* const oldCallback = internal.callback;

        start (0);

        if (oldCallback != 0)
            oldCallback->audioDeviceStopped();
    }

    bool isPlaying()
    {
        return isStarted && internal.error.isEmpty();
    }

    const String getLastError()
    {
        return internal.error;
    }

    String inputId, outputId;

private:
    bool isOpen_, isStarted;
    ALSAThread internal;
};


//==============================================================================
class ALSAAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    ALSAAudioIODeviceType()
        : AudioIODeviceType ("ALSA"),
          hasScanned (false)
    {
    }

    ~ALSAAudioIODeviceType()
    {
    }

    //==============================================================================
    void scanForDevices()
    {
        if (hasScanned)
            return;

        hasScanned = true;
        inputNames.clear();
        inputIds.clear();
        outputNames.clear();
        outputIds.clear();

        snd_ctl_t* handle;
        snd_ctl_card_info_t* info;
        snd_ctl_card_info_alloca (&info);

        int cardNum = -1;

        while (outputIds.size() + inputIds.size() <= 32)
        {
            snd_card_next (&cardNum);

            if (cardNum < 0)
                break;

            if (snd_ctl_open (&handle, ("hw:" + String (cardNum)).toUTF8(), SND_CTL_NONBLOCK) >= 0)
            {
                if (snd_ctl_card_info (handle, info) >= 0)
                {
                    String cardId (snd_ctl_card_info_get_id (info));

                    if (cardId.removeCharacters ("0123456789").isEmpty())
                        cardId = String (cardNum);

                    int device = -1;

                    for (;;)
                    {
                        if (snd_ctl_pcm_next_device (handle, &device) < 0 || device < 0)
                            break;

                        String id, name;
                        id << "hw:" << cardId << ',' << device;

                        bool isInput, isOutput;

                        if (testDevice (id, isInput, isOutput))
                        {
                            name << snd_ctl_card_info_get_name (info);

                            if (name.isEmpty())
                                name = id;

                            if (isInput)
                            {
                                inputNames.add (name);
                                inputIds.add (id);
                            }

                            if (isOutput)
                            {
                                outputNames.add (name);
                                outputIds.add (id);
                            }
                        }
                    }
                }

                snd_ctl_close (handle);
            }
        }

        inputNames.appendNumbersToDuplicates (false, true);
        outputNames.appendNumbersToDuplicates (false, true);
    }

    const StringArray getDeviceNames (bool wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputNames : outputNames;
    }

    int getDefaultDeviceIndex (bool forInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    bool hasSeparateInputsAndOutputs() const    { return true; }

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        ALSAAudioIODevice* d = dynamic_cast <ALSAAudioIODevice*> (device);
        if (d == 0)
            return -1;

        return asInput ? inputIds.indexOf (d->inputId)
                       : outputIds.indexOf (d->outputId);
    }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        const int inputIndex = inputNames.indexOf (inputDeviceName);
        const int outputIndex = outputNames.indexOf (outputDeviceName);

        String deviceName (outputIndex >= 0 ? outputDeviceName
                                            : inputDeviceName);

        if (inputIndex >= 0 || outputIndex >= 0)
            return new ALSAAudioIODevice (deviceName,
                                          inputIds [inputIndex],
                                          outputIds [outputIndex]);

        return 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    StringArray inputNames, outputNames, inputIds, outputIds;
    bool hasScanned;

    static bool testDevice (const String& id, bool& isInput, bool& isOutput)
    {
        unsigned int minChansOut = 0, maxChansOut = 0;
        unsigned int minChansIn = 0, maxChansIn = 0;
        Array <int> rates;

        getDeviceProperties (id, minChansOut, maxChansOut, minChansIn, maxChansIn, rates);

        DBG ("ALSA device: " + id
              + " outs=" + String ((int) minChansOut) + "-" + String ((int) maxChansOut)
              + " ins=" + String ((int) minChansIn) + "-" + String ((int) maxChansIn)
              + " rates=" + String (rates.size()));

        isInput = maxChansIn > 0;
        isOutput = maxChansOut > 0;

        return (isInput || isOutput) && rates.size() > 0;
    }

    ALSAAudioIODeviceType (const ALSAAudioIODeviceType&);
    ALSAAudioIODeviceType& operator= (const ALSAAudioIODeviceType&);
};

//==============================================================================
AudioIODeviceType* juce_createAudioIODeviceType_ALSA()
{
    return new ALSAAudioIODeviceType();
}

#endif

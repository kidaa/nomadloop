# Microsoft Developer Studio Project File - Name="JUCE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=JUCE - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JUCE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JUCE.mak" CFG="JUCE - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JUCE - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "JUCE - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JUCE - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/intermediate_win32/static"
# PROP Intermediate_Dir "../../../bin/intermediate_win32/static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /GR /GX /O2 /D "NDEBUG" /D "WIN32" /D "_LIB" /D "UNICODE" /D "_UNICODE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../bin/jucelib_static_Win32.lib"

!ELSEIF  "$(CFG)" == "JUCE - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/intermediate_win32/staticdebug"
# PROP Intermediate_Dir "../../../bin/intermediate_win32/staticdebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../bin/jucelib_static_Win32_debug.lib"

!ENDIF 

# Begin Target

# Name "JUCE - Win32 Release"
# Name "JUCE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "appframework"

# PROP Default_Filter ""
# Begin Group "application"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\application\juce_Application.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_Application.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandID.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationCommandTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\application\juce_ApplicationProperties.h
# End Source File
# End Group
# Begin Group "audio"

# PROP Default_Filter ""
# Begin Group "audio_file_formats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AiffAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AiffAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioCDReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioCDReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioFormatManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioFormatManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioFormatReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioFormatWriter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioSubsectionReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioSubsectionReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioThumbnail.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioThumbnail.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioThumbnailCache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_AudioThumbnailCache.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_FlacAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_FlacAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_OggVorbisAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_OggVorbisAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_QuickTimeAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_QuickTimeAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_WavAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_file_formats\juce_WavAudioFormat.h
# End Source File
# End Group
# Begin Group "audio_sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioFormatReaderSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioFormatReaderSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioSourcePlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioSourcePlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioTransportSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_AudioTransportSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_BufferingAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_BufferingAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_ChannelRemappingAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_ChannelRemappingAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_MixerAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_MixerAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_PositionableAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_ResamplingAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_ResamplingAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_ToneGeneratorAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\audio_sources\juce_ToneGeneratorAudioSource.h
# End Source File
# End Group
# Begin Group "devices"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_AudioDeviceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_AudioDeviceManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_AudioIODevice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_AudioIODevice.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_AudioIODeviceType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_AudioIODeviceType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_MidiInput.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_MidiOutput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\devices\juce_MidiOutput.h
# End Source File
# End Group
# Begin Group "midi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiKeyboardState.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiKeyboardState.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiMessageCollector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiMessageCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiMessageSequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\midi\juce_MidiMessageSequence.h
# End Source File
# End Group
# Begin Group "synthesisers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\synthesisers\juce_Sampler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\synthesisers\juce_Sampler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\synthesisers\juce_Synthesiser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\synthesisers\juce_Synthesiser.h
# End Source File
# End Group
# Begin Group "dsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\dsp\juce_AudioDataConverters.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\dsp\juce_AudioDataConverters.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\dsp\juce_AudioSampleBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\dsp\juce_AudioSampleBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\dsp\juce_IIRFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\dsp\juce_IIRFilter.h
# End Source File
# End Group
# Begin Group "processors"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioPlayHead.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorGraph.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_AudioProcessorPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_GenericAudioProcessorEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\processors\juce_GenericAudioProcessorEditor.h
# End Source File
# End Group
# Begin Group "plugins"

# PROP Default_Filter ""
# Begin Group "formats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\formats\juce_DirectXPluginFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\formats\juce_LADSPAPluginFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\formats\juce_VSTPluginFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\formats\juce_VSTPluginFormat.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_AudioPluginFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_AudioPluginFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_AudioPluginFormatManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_AudioPluginFormatManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_AudioPluginInstance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_AudioPluginInstance.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_KnownPluginList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_KnownPluginList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_PluginDescription.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_PluginDescription.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_PluginDirectoryScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_PluginDirectoryScanner.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_PluginListComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\audio\plugins\juce_PluginListComponent.h
# End Source File
# End Group
# End Group
# Begin Group "events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\events\juce_ActionBroadcaster.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ActionBroadcaster.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ActionListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ActionListenerList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ActionListenerList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_AsyncUpdater.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_AsyncUpdater.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ChangeBroadcaster.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ChangeBroadcaster.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ChangeListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ChangeListenerList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ChangeListenerList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_InterprocessConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_InterprocessConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_InterprocessConnectionServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_InterprocessConnectionServer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_ListenerList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_Message.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_Message.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_MessageListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_MessageListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_MessageManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_MessageManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_MultiTimer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_MultiTimer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_Timer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\events\juce_Timer.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Group "graphics"

# PROP Default_Filter ""
# Begin Group "colour"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_Colour.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_Colour.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_ColourGradient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_ColourGradient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_Colours.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_Colours.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\colour\juce_PixelFormats.h
# End Source File
# End Group
# Begin Group "contexts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_EdgeTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_EdgeTable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_FillType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_FillType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_Graphics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_Graphics.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_Justification.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_Justification.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_LowLevelGraphicsContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_LowLevelGraphicsPostScriptRenderer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_LowLevelGraphicsPostScriptRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_LowLevelGraphicsSoftwareRenderer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_LowLevelGraphicsSoftwareRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_RectanglePlacement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\contexts\juce_RectanglePlacement.h
# End Source File
# End Group
# Begin Group "fonts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_Font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_Font.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_GlyphArrangement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_GlyphArrangement.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_TextLayout.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_TextLayout.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_Typeface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\fonts\juce_TypeFace.h
# End Source File
# End Group
# Begin Group "geometry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_AffineTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_AffineTransform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_BorderSize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_BorderSize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_Line.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_Line.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_Path.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_Path.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_PathIterator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_PathIterator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_PathStrokeType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_PathStrokeType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_Point.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_PositionedRectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_PositionedRectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_Rectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_RectangleList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\geometry\juce_RectangleList.h
# End Source File
# End Group
# Begin Group "imaging"

# PROP Default_Filter ""
# Begin Group "image_file_formats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\image_file_formats\juce_GIFLoader.cpp
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\image_file_formats\juce_JPEGLoader.cpp
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\image_file_formats\juce_PNGLoader.cpp
# ADD CPP /W1
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_Image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_Image.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_ImageCache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_ImageCache.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_ImageConvolutionKernel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_ImageConvolutionKernel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_ImageFileFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\imaging\juce_ImageFileFormat.h
# End Source File
# End Group
# Begin Group "effects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_DropShadowEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_DropShadowEffect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_GlowEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_GlowEffect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_ImageEffectFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_ReduceOpacityEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\effects\juce_ReduceOpacityEffect.h
# End Source File
# End Group
# Begin Group "drawables"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_Drawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_Drawable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawableComposite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawableComposite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawableImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawableImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawablePath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawablePath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawableText.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_DrawableText.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\graphics\drawables\juce_SVGParser.cpp
# End Source File
# End Group
# End Group
# Begin Group "components"

# PROP Default_Filter ""
# Begin Group "buttons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ArrowButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ArrowButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_Button.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_Button.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_DrawableButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_DrawableButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_HyperlinkButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_HyperlinkButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ImageButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ImageButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ShapeButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ShapeButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_TextButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_TextButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ToggleButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ToggleButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ToolbarButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\buttons\juce_ToolbarButton.h
# End Source File
# End Group
# Begin Group "controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ComboBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ComboBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_Label.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_Label.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ProgressBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_Slider.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_Slider.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TableHeaderComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TableHeaderComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TableListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TableListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TextEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TextEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_Toolbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_Toolbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ToolbarItemComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ToolbarItemComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ToolbarItemFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ToolbarItemPalette.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_ToolbarItemPalette.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TreeView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\controls\juce_TreeView.h
# End Source File
# End Group
# Begin Group "keyboard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyboardFocusTraverser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyboardFocusTraverser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyMappingEditorComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyMappingEditorComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyPress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyPress.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyPressMappingSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_KeyPressMappingSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_ModifierKeys.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\keyboard\juce_ModifierKeys.h
# End Source File
# End Group
# Begin Group "layout"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ComponentAnimator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ComponentAnimator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ComponentBoundsConstrainer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ComponentBoundsConstrainer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ComponentMovementWatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ComponentMovementWatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_GroupComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_GroupComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_MultiDocumentPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_MultiDocumentPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ResizableBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ResizableBorderComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ResizableBorderComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ResizableCornerComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ResizableCornerComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ScrollBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_ScrollBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_StretchableLayoutManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_StretchableLayoutManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_StretchableLayoutResizerBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_StretchableLayoutResizerBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_StretchableObjectResizer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_StretchableObjectResizer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_TabbedButtonBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_TabbedButtonBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_TabbedComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_TabbedComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_Viewport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\layout\juce_Viewport.h
# End Source File
# End Group
# Begin Group "lookandfeel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\lookandfeel\juce_LookAndFeel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\lookandfeel\juce_LookAndFeel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\lookandfeel\juce_OldSchoolLookAndFeel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\lookandfeel\juce_OldSchoolLookAndFeel.h
# End Source File
# End Group
# Begin Group "menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_MenuBarComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_MenuBarComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_MenuBarModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_MenuBarModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_PopupMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_PopupMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\menus\juce_PopupMenuCustomComponent.h
# End Source File
# End Group
# Begin Group "mouse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_ComponentDragger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_ComponentDragger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_DragAndDropContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_DragAndDropContainer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_DragAndDropTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_FileDragAndDropTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_LassoComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseCursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseEvent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseHoverDetector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseHoverDetector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseInputSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseInputSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_MouseListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\mouse\juce_TooltipClient.h
# End Source File
# End Group
# Begin Group "special"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_ActiveXControlComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_AudioDeviceSelectorComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_AudioDeviceSelectorComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_BubbleComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_BubbleComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_BubbleMessageComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_BubbleMessageComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_ColourSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_ColourSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_DropShadower.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_DropShadower.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_MagnifierComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_MagnifierComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_MidiKeyboardComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_MidiKeyboardComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_OpenGLComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_OpenGLComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_PreferencesPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_PreferencesPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_SystemTrayIconComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\special\juce_SystemTrayIconComponent.h
# End Source File
# End Group
# Begin Group "windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_AlertWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_AlertWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ComponentPeer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ComponentPeer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_DialogWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_DialogWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_DocumentWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_DocumentWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ResizableWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ResizableWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_SplashScreen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ThreadWithProgressWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ThreadWithProgressWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ToolTipWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_ToolTipWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_TopLevelWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\windows\juce_TopLevelWindow.h
# End Source File
# End Group
# Begin Group "filebrowser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_DirectoryContentsDisplayComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_DirectoryContentsDisplayComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_DirectoryContentsList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_DirectoryContentsList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileBrowserComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileBrowserComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileBrowserListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileChooser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileChooser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileChooserDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileChooserDialogBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileListComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileListComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FilenameComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FilenameComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FilePreviewComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileSearchPathListComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileSearchPathListComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileTreeComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_FileTreeComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_ImagePreviewComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_ImagePreviewComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_WildcardFileFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\filebrowser\juce_WildcardFileFilter.h
# End Source File
# End Group
# Begin Group "properties"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_BooleanPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_BooleanPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_ButtonPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_ButtonPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_ChoicePropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_ChoicePropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_PropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_PropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_PropertyPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_PropertyPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_SliderPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_SliderPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_TextPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\properties\juce_TextPropertyComponent.h
# End Source File
# End Group
# Begin Group "codeeditor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CodeDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CodeDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CodeEditorComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CodeEditorComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CodeTokeniser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CPlusPlusCodeTokeniser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\code_editor\juce_CPlusPlusCodeTokeniser.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_Component.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_Component.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_ComponentDeletionWatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_ComponentListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_ComponentListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_Desktop.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gui\components\juce_Desktop.h
# End Source File
# End Group
# End Group
# Begin Group "documents"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_DeletedAtShutdown.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_DeletedAtShutdown.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_FileBasedDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_FileBasedDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_PropertiesFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_PropertiesFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_RecentlyOpenedFilesList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_RecentlyOpenedFilesList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_SelectedItemSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_SystemClipboard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_UndoableAction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_UndoManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utilities\juce_UndoManager.h
# End Source File
# End Group
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Group "basics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\core\juce_Atomic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_DataConversions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_FileLogger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_FileLogger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Initialisation.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Logger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_MathsFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Memory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_PerformanceCounter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_PerformanceCounter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_PlatformDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_PlatformUtilities.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Random.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Random.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_RelativeTime.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_RelativeTime.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Singleton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_StandardHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_SystemStats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_SystemStats.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_TargetPlatform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Time.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Time.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Uuid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\core\juce_Uuid.h
# End Source File
# End Group
# Begin Group "containers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\containers\juce_Array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_ArrayAllocationBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_BitArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_BitArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_ElementComparator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_MemoryBlock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_MemoryBlock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_NamedValueSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_NamedValueSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_OwnedArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_PropertySet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_PropertySet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_ReferenceCountedArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_ReferenceCountedObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_SortedSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_SparseSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_Value.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_Value.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_ValueTree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_ValueTree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_Variant.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_Variant.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\containers\juce_VoidArray.h
# End Source File
# End Group
# Begin Group "io"

# PROP Default_Filter ""
# Begin Group "files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_DirectoryIterator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_DirectoryIterator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_File.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_File.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_FileInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_FileInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_FileOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_FileOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_FileSearchPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_FileSearchPath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_NamedPipe.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_NamedPipe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_TemporaryFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_TemporaryFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_ZipFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\files\juce_ZipFile.h
# End Source File
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\io\network\juce_Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\network\juce_Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\network\juce_URL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\network\juce_URL.h
# End Source File
# End Group
# Begin Group "streams"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_BufferedInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_BufferedInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_GZIPCompressorOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_GZIPCompressorOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_GZIPDecompressorInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_GZIPDecompressorInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_InputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_InputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_MemoryInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_MemoryInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_MemoryOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_MemoryOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_OutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_OutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_SubregionStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_SubregionStream.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_FileInputSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_FileInputSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\io\streams\juce_InputSource.h
# End Source File
# End Group
# Begin Group "text"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\text\juce_CharacterFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_CharacterFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_LocalisedStrings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_LocalisedStrings.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_String.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_String.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_StringArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_StringArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_StringPairArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_StringPairArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_TextFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_XmlDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_XmlDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_XmlElement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\text\juce_XmlElement.h
# End Source File
# End Group
# Begin Group "threads"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\threads\juce_CriticalSection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_InterProcessLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_Process.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ReadWriteLock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ReadWriteLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ScopedLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ScopedReadLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ScopedTryLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ScopedWriteLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_ThreadPool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_TimeSliceThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_TimeSliceThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\threads\juce_WaitableEvent.h
# End Source File
# End Group
# Begin Group "cryptography"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_BlowFish.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_BlowFish.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_MD5.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_MD5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_Primes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_Primes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_RSAKey.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cryptography\juce_RSAKey.h
# End Source File
# End Group
# End Group
# Begin Group "win32_code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_ActiveXComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_ASIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_AudioCDReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_DirectSound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_DynamicLibraryLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_FileChooser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Files.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Fonts.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Messaging.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Midi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Misc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\juce_win32_NativeCode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Network.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_OpenGLComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_PlatformUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_QuickTimeMovieComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_SystemStats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Threads.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_WebBrowserComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\juce_win32_Windowing.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\native\windows\win32_headers.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\juce.h
# End Source File
# Begin Source File

SOURCE=..\..\..\juce_Config.h
# End Source File
# End Group
# Begin Source File

SOURCE="..\..\..\docs\JUCE changelist.txt"
# End Source File
# End Target
# End Project

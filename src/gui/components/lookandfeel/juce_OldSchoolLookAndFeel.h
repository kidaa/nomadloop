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

#ifndef __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__
#define __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__

#include "juce_LookAndFeel.h"


//==============================================================================
/**
    The original Juce look-and-feel.

*/
class JUCE_API  OldSchoolLookAndFeel    : public LookAndFeel
{
public:
    //==============================================================================
    /** Creates the default JUCE look and feel. */
    OldSchoolLookAndFeel();

    /** Destructor. */
    virtual ~OldSchoolLookAndFeel();

    //==============================================================================
    /** Draws the lozenge-shaped background for a standard button. */
    virtual void drawButtonBackground (Graphics& g,
                                       Button& button,
                                       const Colour& backgroundColour,
                                       bool isMouseOverButton,
                                       bool isButtonDown);


    /** Draws the contents of a standard ToggleButton. */
    virtual void drawToggleButton (Graphics& g,
                                   ToggleButton& button,
                                   bool isMouseOverButton,
                                   bool isButtonDown);

    virtual void drawTickBox (Graphics& g,
                              Component& component,
                              float x, float y, float w, float h,
                              const bool ticked,
                              const bool isEnabled,
                              const bool isMouseOverButton,
                              const bool isButtonDown);

    //==============================================================================
    virtual void drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                  int width, int height,
                                  double progress, const String& textToShow);

    //==============================================================================
    virtual void drawScrollbarButton (Graphics& g,
                                      ScrollBar& scrollbar,
                                      int width, int height,
                                      int buttonDirection,
                                      bool isScrollbarVertical,
                                      bool isMouseOverButton,
                                      bool isButtonDown);

    virtual void drawScrollbar (Graphics& g,
                                ScrollBar& scrollbar,
                                int x, int y,
                                int width, int height,
                                bool isScrollbarVertical,
                                int thumbStartPosition,
                                int thumbSize,
                                bool isMouseOver,
                                bool isMouseDown);

    virtual ImageEffectFilter* getScrollbarEffect();

    //==============================================================================
    virtual void drawTextEditorOutline (Graphics& g,
                                        int width, int height,
                                        TextEditor& textEditor);

    //==============================================================================
    /** Fills the background of a popup menu component. */
    virtual void drawPopupMenuBackground (Graphics& g, int width, int height);

    virtual void drawMenuBarBackground (Graphics& g, int width, int height,
                                        bool isMouseOverBar,
                                        MenuBarComponent& menuBar);

    //==============================================================================
    virtual void drawComboBox (Graphics& g, int width, int height,
                               const bool isButtonDown,
                               int buttonX, int buttonY,
                               int buttonW, int buttonH,
                               ComboBox& box);

    virtual const Font getComboBoxFont (ComboBox& box);

    //==============================================================================
    virtual void drawLinearSlider (Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle style,
                                   Slider& slider);

    virtual int getSliderThumbRadius (Slider& slider);

    virtual Button* createSliderButton (const bool isIncrement);

    virtual ImageEffectFilter* getSliderEffect();

    //==============================================================================
    virtual void drawCornerResizer (Graphics& g,
                                    int w, int h,
                                    bool isMouseOver,
                                    bool isMouseDragging);

    virtual Button* createDocumentWindowButton (int buttonType);

    virtual void positionDocumentWindowButtons (DocumentWindow& window,
                                                int titleBarX, int titleBarY,
                                                int titleBarW, int titleBarH,
                                                Button* minimiseButton,
                                                Button* maximiseButton,
                                                Button* closeButton,
                                                bool positionTitleBarButtonsOnLeft);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    DropShadowEffect scrollbarShadow;

    OldSchoolLookAndFeel (const OldSchoolLookAndFeel&);
    const OldSchoolLookAndFeel& operator= (const OldSchoolLookAndFeel&);
};


#endif   // __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__

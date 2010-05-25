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

#ifndef __JUCER_DRAWABLEDOCUMENT_JUCEHEADER__
#define __JUCER_DRAWABLEDOCUMENT_JUCEHEADER__

#include "../../jucer_Headers.h"
#include "../Project/jucer_Project.h"
#include "../../utility/jucer_MarkerListBase.h"


//==============================================================================
class DrawableDocument  :  public ValueTree::Listener,
                           public ChangeBroadcaster,
                           public Drawable::ImageProvider
{
public:
    //==============================================================================
    DrawableDocument (Project* project);
    ~DrawableDocument();

    //==============================================================================
    void setName (const String& name);
    const String getName() const;

    bool reload (const File& drawableFile);
    bool save (const File& drawableFile);
    bool hasChangedSinceLastSave() const;
    void changed();

    ValueTree& getRoot()                    { return root; }
    DrawableComposite::ValueTreeWrapper getRootDrawableNode() const;

//    Value getCanvasWidth() const            { return getRootValueNonUndoable (Ids::width); }
  //  Value getCanvasHeight() const           { return getRootValueNonUndoable (Ids::height); }

    ValueTree findDrawableState (const String& objectId, bool recursive) const;
    const String createUniqueID (const String& suggestion) const;

    void createItemProperties (Array <PropertyComponent*>& props, const StringArray& selectedItemIds);

    void addNewItemMenuItems (PopupMenu& menu) const;
    const ValueTree performNewItemMenuItem (int menuResultCode);

    //==============================================================================
    class MarkerList    : public MarkerListBase
    {
    public:
        MarkerList (DrawableDocument& document, bool isX);
        ~MarkerList() {}

        const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;
        bool createProperties (Array <PropertyComponent*>& props, const String& itemId);
        void addMarkerMenuItems (const ValueTree& markerState, const RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1);
        const String getChosenMarkerMenuItem (const RelativeCoordinate& coord, int itemId) const;
        UndoManager* getUndoManager() const;
        const String getNonexistentMarkerName (const String& name);
        void renameAnchor (const String& oldName, const String& newName);

    private:
        DrawableDocument& document;

        MarkerList (const MarkerList&);
        MarkerList& operator= (const MarkerList&);
    };

    MarkerList& getMarkerListX() const            { return *markersX; }
    MarkerList& getMarkerListY() const            { return *markersY; }
    MarkerList& getMarkerList (bool isX) const    { return isX ? *markersX : *markersY; }

    const String getNonexistentMarkerName (const String& name);
    void renameAnchor (const String& oldName, const String& newName);

    Image* getImageForIdentifier (const var& imageIdentifier);
    const var getIdentifierForImage (Image* image);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& name);
    void valueTreeChildrenChanged (ValueTree& tree);
    void valueTreeParentChanged (ValueTree& tree);

    //==============================================================================
    UndoManager* getUndoManager() const throw()               { return &undoManager; }

private:
    Project* project;
    ValueTree root;
    ScopedPointer<MarkerList> markersX, markersY;
    mutable UndoManager undoManager;
    bool saveAsXml, needsSaving;

    void checkRootObject();
    void recursivelyUpdateIDs (Drawable::ValueTreeWrapperBase& d);

    Value getRootValueUndoable (const Identifier& name) const        { return root.getPropertyAsValue (name, getUndoManager()); }
    Value getRootValueNonUndoable (const Identifier& name) const     { return root.getPropertyAsValue (name, 0); }

    void save (OutputStream& output);
    bool load (InputStream& input);

    bool createItemProperties (Array <PropertyComponent*>& props, const String& itemId);

    const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;
    void addMarkerMenuItem (int i, const RelativeCoordinate& coord, const String& objectName, const String& edge,
                            PopupMenu& menu, bool isAnchor1, const String& fullCoordName);
};


#endif   // __JUCER_DRAWABLEDOCUMENT_JUCEHEADER__

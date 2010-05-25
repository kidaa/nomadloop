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

#include "jucer_DrawableDocument.h"
#include "jucer_DrawableTypeHandler.h"


//==============================================================================
class DrawableTypeManager   : public DeletedAtShutdown
{
public:
    DrawableTypeManager()
    {
        handlers.add (new DrawablePathHandler());
        handlers.add (new DrawableImageHandler());
        handlers.add (new DrawableCompositeHandler());
    }

    ~DrawableTypeManager()
    {
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (DrawableTypeManager);

    //==============================================================================
    int getNumHandlers() const                                      { return handlers.size(); }
    DrawableTypeHandler* getHandler (const int index) const         { return handlers[index]; }

    DrawableTypeHandler* getHandlerFor (const Identifier& type)
    {
        for (int i = handlers.size(); --i >= 0;)
            if (handlers.getUnchecked(i)->getValueTreeType() == type)
                return handlers.getUnchecked(i);

        jassertfalse;
        return 0;
    }

private:
    OwnedArray <DrawableTypeHandler> handlers;
};

juce_ImplementSingleton_SingleThreaded (DrawableTypeManager);


//==============================================================================
DrawableTypeInstance::DrawableTypeInstance (DrawableDocument& document_, const ValueTree& state_)
    : document (document_), state (state_)
{
}

Value DrawableTypeInstance::getValue (const Identifier& name) const
{
    return state.getPropertyAsValue (name, document.getUndoManager());
}

void DrawableTypeInstance::createProperties (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getValue (Drawable::ValueTreeWrapperBase::idProperty), "Object ID", 128, false));

    getHandler()->createPropertyEditors (*this, props);
}

DrawableTypeHandler* DrawableTypeInstance::getHandler() const
{
    DrawableTypeHandler* h = DrawableTypeManager::getInstance()->getHandlerFor (state.getType());
    jassert (h != 0);
    return h;
}

bool DrawableTypeInstance::setBounds (Drawable* drawable, const Rectangle<float>& newBounds)
{
    return getHandler()->setBounds (*this, drawable, newBounds);
}


//==============================================================================
namespace Tags
{
    const Identifier drawableTag ("DRAWABLE");
    const Identifier markersGroupXTag ("MARKERS_X");
    const Identifier markersGroupYTag ("MARKERS_Y");
}

//==============================================================================
DrawableDocument::DrawableDocument (Project* project_)
    : project (project_),
      root (Tags::drawableTag),
      saveAsXml (true),
      needsSaving (false)
{
    DrawableComposite dc;
    root.addChild (dc.createValueTree (0), -1, 0);

    setName ("Drawable");
    checkRootObject();

    root.addListener (this);
}

DrawableDocument::~DrawableDocument()
{
    root.removeListener (this);
}

void DrawableDocument::recursivelyUpdateIDs (Drawable::ValueTreeWrapperBase& d)
{
    if (d.getID().isEmpty())
        d.setID (createUniqueID (d.getState().getType().toString().toLowerCase() + "1"), 0);

    if (d.getState().getType() == DrawableComposite::valueTreeType)
    {
        const DrawableComposite::ValueTreeWrapper composite (d.getState());

        for (int i = 0; i < composite.getNumDrawables(); ++i)
        {
            Drawable::ValueTreeWrapperBase child (composite.getDrawableState (i));
            recursivelyUpdateIDs (child);
        }
    }
}

void DrawableDocument::checkRootObject()
{
    if (! root.hasProperty (Ids::id_))
        root.setProperty (Ids::id_, createAlphaNumericUID(), 0);

    if (markersX == 0)
        markersX = new MarkerList (*this, true);

    if (markersY == 0)
        markersY = new MarkerList (*this, false);

/*    if ((int) getCanvasWidth().getValue() <= 0)
        getCanvasWidth() = 500;

    if ((int) getCanvasHeight().getValue() <= 0)
        getCanvasHeight() = 500;
*/
    DrawableComposite::ValueTreeWrapper rootObject (getRootDrawableNode());
    recursivelyUpdateIDs (rootObject);
}

//==============================================================================
void DrawableDocument::setName (const String& name)
{
    root.setProperty (Ids::name, name, getUndoManager());
}

const String DrawableDocument::getName() const
{
    return root [Ids::name];
}

bool DrawableDocument::hasChangedSinceLastSave() const
{
    return needsSaving;
}

bool DrawableDocument::reload (const File& drawableFile)
{
    ScopedPointer <InputStream> stream (drawableFile.createInputStream());

    if (stream != 0 && load (*stream))
    {
        checkRootObject();
        undoManager.clearUndoHistory();
        needsSaving = false;
        return true;
    }

    return false;
}

bool DrawableDocument::save (const File& drawableFile)
{
    TemporaryFile tempFile (drawableFile);
    ScopedPointer <OutputStream> out (tempFile.getFile().createOutputStream());

    if (out == 0)
        return false;

    save (*out);

    needsSaving = ! tempFile.overwriteTargetFileWithTemporary();
    return ! needsSaving;
}

void DrawableDocument::save (OutputStream& output)
{
    if (saveAsXml)
    {
        ScopedPointer <XmlElement> xml (root.createXml());
        jassert (xml != 0);

        if (xml != 0)
            xml->writeToStream (output, String::empty, false, false);
    }
    else
    {
        root.writeToStream (output);
    }
}

bool DrawableDocument::load (InputStream& input)
{
    int64 originalPos = input.getPosition();
    ValueTree loadedTree ("dummy");

    XmlDocument xmlDoc (input.readEntireStreamAsString());
    ScopedPointer <XmlElement> xml (xmlDoc.getDocumentElement());

    if (xml != 0)
    {
        loadedTree = ValueTree::fromXml (*xml);
    }
    else
    {
        input.setPosition (originalPos);
        loadedTree = ValueTree::readFromStream (input);
    }

    if (loadedTree.hasType (Tags::drawableTag))
    {
        root.removeListener (this);
        root = loadedTree;
        root.addListener (this);

        valueTreeParentChanged (loadedTree);

        needsSaving = false;
        undoManager.clearUndoHistory();

        return true;
    }

    return false;
}

void DrawableDocument::changed()
{
    needsSaving = true;
    sendChangeMessage (this);
}

DrawableComposite::ValueTreeWrapper DrawableDocument::getRootDrawableNode() const
{
    return DrawableComposite::ValueTreeWrapper (root.getChild (0));
}

ValueTree DrawableDocument::findDrawableState (const String& objectId, bool recursive) const
{
    return getRootDrawableNode().getDrawableWithId (objectId, recursive);
}

const String DrawableDocument::createUniqueID (const String& name) const
{
    String n (CodeHelpers::makeValidIdentifier (name, false, true, false));
    int suffix = 2;

    while (markersX->getMarkerNamed (n).isValid() || markersY->getMarkerNamed (n).isValid()
            || findDrawableState (n, true).isValid())
        n = n.trimCharactersAtEnd ("0123456789") + String (suffix++);

    return n;
}

bool DrawableDocument::createItemProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree drawable (findDrawableState (itemId, false));

    if (drawable.isValid())
    {
        DrawableTypeInstance item (*this, drawable);
        item.createProperties (props);
        return true;
    }

    if (markersX->createProperties (props, itemId)
         || markersY->createProperties (props, itemId))
        return true;

    return false;
}

void DrawableDocument::createItemProperties (Array <PropertyComponent*>& props, const StringArray& selectedItemIds)
{
    if (selectedItemIds.size() != 1)
        return; //xxx

    for (int i = 0; i < selectedItemIds.size(); ++i)
        createItemProperties (props, selectedItemIds[i]);
}

//==============================================================================
const int menuItemOffset = 0x63451fa4;

void DrawableDocument::addNewItemMenuItems (PopupMenu& menu) const
{
    DrawableTypeManager* const typeMan = DrawableTypeManager::getInstance();

    for (int i = 0; i < typeMan->getNumHandlers(); ++i)
        if (typeMan->getHandler(i)->canBeCreated)
            menu.addItem (i + menuItemOffset, "New " + typeMan->getHandler(i)->getDisplayName());
}

const ValueTree DrawableDocument::performNewItemMenuItem (int menuResultCode)
{
    DrawableTypeManager* const typeMan = DrawableTypeManager::getInstance();

    if (menuResultCode >= menuItemOffset && menuResultCode < menuItemOffset + typeMan->getNumHandlers())
    {
        DrawableTypeHandler* handler = typeMan->getHandler (menuResultCode - menuItemOffset);
        jassert (handler != 0);

        if (handler != 0)
        {
            ValueTree state (handler->createNewInstance (*this,
                                                         Point<float> (Random::getSystemRandom().nextFloat() * 100.0f + 100.0f,
                                                                       Random::getSystemRandom().nextFloat() * 100.0f + 100.0f)));

            Drawable::ValueTreeWrapperBase wrapper (state);
            recursivelyUpdateIDs (wrapper);

            getRootDrawableNode().addDrawable (state, -1, getUndoManager());

            return state;
        }
    }

    return ValueTree::invalid;
}

//==============================================================================
Image* DrawableDocument::getImageForIdentifier (const var& imageIdentifier)
{
    return ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize);
}

const var DrawableDocument::getIdentifierForImage (Image* image)
{
    return var::null; //xxx todo
}

//==============================================================================
void DrawableDocument::valueTreePropertyChanged (ValueTree& tree, const Identifier& name)
{
    changed();
}

void DrawableDocument::valueTreeChildrenChanged (ValueTree& tree)
{
    changed();
}

void DrawableDocument::valueTreeParentChanged (ValueTree& tree)
{
    changed();
}

//==============================================================================
const RelativeCoordinate DrawableDocument::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == "parent")
    {
//        if (edge == "right")     return RelativeCoordinate ((double) getCanvasWidth().getValue(), true);
  //      if (edge == "bottom")    return RelativeCoordinate ((double) getCanvasHeight().getValue(), false);
    }

    if (objectName.isNotEmpty() && edge.isNotEmpty())
    {
/*            const ValueTree comp (getComponentWithMemberName (compName));

        if (comp.isValid())
        {
            const RelativeRectangle coords (getCoordsFor (comp));

            if (edge == RelativeCoordinate::leftName)   return coords.left;
            if (edge == "right")  return coords.right;
            if (edge == "top")    return coords.top;
            if (edge == "bottom") return coords.bottom;
        }*/
    }

    {
        const ValueTree marker (getMarkerListX().getMarkerNamed (objectName));
        if (marker.isValid())
            return getMarkerListX().getCoordinate (marker);
    }

    {
        const ValueTree marker (getMarkerListY().getMarkerNamed (objectName));
        if (marker.isValid())
            return getMarkerListY().getCoordinate (marker);
    }

    return RelativeCoordinate();
}

//==============================================================================
DrawableDocument::MarkerList::MarkerList (DrawableDocument& document_, bool isX_)
    : MarkerListBase (document_.getRoot().getChildWithName (isX_ ? Tags::markersGroupXTag : Tags::markersGroupYTag), isX_),
      document (document_)
{
}

const RelativeCoordinate DrawableDocument::MarkerList::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == "parent")
    {
//        if (edge == "right")     return RelativeCoordinate ((double) document.getCanvasWidth().getValue(), true);
  //      if (edge == "bottom")    return RelativeCoordinate ((double) document.getCanvasHeight().getValue(), false);
    }

    const ValueTree marker (getMarkerNamed (objectName));
    if (marker.isValid())
        return getCoordinate (marker);

    return RelativeCoordinate();
}

bool DrawableDocument::MarkerList::createProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree marker (group.getChildWithProperty (getIdProperty(), itemId));

    if (marker.isValid())
    {
        props.add (new TextPropertyComponent (getNameAsValue (marker), "Marker Name", 256, false));
//        props.add (new MarkerPositionComponent (document, "Position", marker,
  //                                              marker.getPropertyAsValue (markerPosProperty, document.getUndoManager()),
    //                                            contains (marker)));
        return true;
    }

    return false;
}

void DrawableDocument::addMarkerMenuItem (int i, const RelativeCoordinate& coord, const String& objectName, const String& edge, PopupMenu& menu,
                                          bool isAnchor1, const String& fullCoordName)
{
//    RelativeCoordinate requestedCoord (findNamedCoordinate (objectName, edge, coord.isHorizontal()));

//    menu.addItem (i, name,
  //                ! (name == fullCoordName || requestedCoord.referencesIndirectly (fullCoordName, *this)),
    //              name == (isAnchor1 ? coord.getAnchor1() : coord.getAnchor2()));
}

void DrawableDocument::MarkerList::addMarkerMenuItems (const ValueTree& markerState, const RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1)
{
/*    const String fullCoordName (getName (markerState));

    if (coord.isHorizontal())
    {
        document.addMarkerMenuItem (1, coord, "parent", "left", menu, isAnchor1, fullCoordName);
        document.addMarkerMenuItem (2, coord, "parent", "right", menu, isAnchor1, fullCoordName);
    }
    else
    {
        document.addMarkerMenuItem (1, coord, "parent", "top", menu, isAnchor1, fullCoordName);
        document.addMarkerMenuItem (2, coord, "parent", "bottom", menu, isAnchor1, fullCoordName);
    }

    menu.addSeparator();
    const MarkerList& markerList = document.getMarkerList (coord.isHorizontal());

    for (int i = 0; i < markerList.size(); ++i)
        document.addMarkerMenuItem (100 + i, coord, markerList.getName (markerList.getMarker (i)),
                                    String::empty, menu, isAnchor1, fullCoordName);*/
}

const String DrawableDocument::MarkerList::getChosenMarkerMenuItem (const RelativeCoordinate& coord, int i) const
{
/*    if (i == 1)  return coord.isHorizontal() ? "parent.left" : "parent.top";
    if (i == 2)  return coord.isHorizontal() ? "parent.right" : "parent.bottom";

    const MarkerList& markerList = document.getMarkerList (coord.isHorizontal());

    if (i >= 100 && i < 10000)
        return markerList.getName (markerList.getMarker (i - 100));

    jassertfalse;*/
    return String::empty;
}

UndoManager* DrawableDocument::MarkerList::getUndoManager() const
{
    return document.getUndoManager();
}

const String DrawableDocument::MarkerList::getNonexistentMarkerName (const String& name)
{
    return document.getNonexistentMarkerName (name);
}

const String DrawableDocument::getNonexistentMarkerName (const String& name)
{
    String n (CodeHelpers::makeValidIdentifier (name, false, true, false));
    int suffix = 2;

    while (markersX->getMarkerNamed (n).isValid() || markersY->getMarkerNamed (n).isValid())
        n = n.trimCharactersAtEnd ("0123456789") + String (suffix++);

    return n;
}

void DrawableDocument::MarkerList::renameAnchor (const String& oldName, const String& newName)
{
    document.renameAnchor (oldName, newName);
}

void DrawableDocument::renameAnchor (const String& oldName, const String& newName)
{
}

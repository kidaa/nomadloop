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

#ifndef __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__
#define __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__


//==============================================================================
/**
*/
class DrawableTreeViewItem  : public JucerTreeViewBase,
                              public ValueTree::Listener,
                              public ChangeListener
{
    DrawableTreeViewItem (DrawableEditor& editor_, const ValueTree& drawableRoot, const String& typeName_)
        : editor (editor_), node (drawableRoot), typeName (typeName_)
    {
        node.addListener (this);
        editor.getSelection().addChangeListener (this);
    }

public:
    static DrawableTreeViewItem* createItemForNode (DrawableEditor& editor, const ValueTree& drawableRoot)
    {
        const char* typeName = 0;

        {
            ScopedPointer <Drawable> d (Drawable::createFromValueTree (drawableRoot));

            if (d != 0)
            {
                if (dynamic_cast <DrawablePath*> ((Drawable*) d) != 0)
                    typeName = "Path";
                else if (dynamic_cast <DrawableImage*> ((Drawable*) d) != 0)
                    typeName = "Image";
                else if (dynamic_cast <DrawableComposite*> ((Drawable*) d) != 0)
                    typeName = "Group";
                else if (dynamic_cast <DrawableText*> ((Drawable*) d) != 0)
                    typeName = "Text";
                else
                {
                    jassertfalse
                }
            }
        }

        if (typeName != 0)
            return new DrawableTreeViewItem (editor, drawableRoot, typeName);

        return 0;
    }

    ~DrawableTreeViewItem()
    {
        editor.getSelection().removeChangeListener (this);
        node.removeListener (this);
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
    {
    }

    void valueTreeChildrenChanged (ValueTree& tree)
    {
        if (tree == node)
            refreshSubItems();
    }

    void valueTreeParentChanged (ValueTree& tree)
    {
    }

    //==============================================================================
    // TreeViewItem stuff..
    bool mightContainSubItems()
    {
        return node.getNumChildren() > 0;
    }

    const String getUniqueName() const
    {
        jassert (node [Ids::id_].toString().isNotEmpty());
        return node [Ids::id_];
    }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
            refreshSubItems();
    }

    void refreshSubItems()
    {
        ScopedPointer <XmlElement> oldOpenness (getOpennessState());

        clearSubItems();

        for (int i = 0; i < node.getNumChildren(); ++i)
        {
            ValueTree subNode (node.getChild (i));
            DrawableTreeViewItem* const item = createItemForNode (editor, subNode);

            if (item != 0)
                addSubItem (item);
        }

        if (oldOpenness != 0)
            restoreOpennessState (*oldOpenness);

        editor.getSelection().changed();
    }

    const String getDisplayName() const
    {
        const String name (getRenamingName());
        return typeName + (name.isEmpty() ? String::empty
                                          : (" \"" + name + "\""));
    }

    const String getRenamingName() const
    {
        return node ["name"];
    }

    void setName (const String& newName)
    {
    }

    bool isMissing()            { return false; }

    Image* getIcon() const
    {
        return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage();
    }

    void itemClicked (const MouseEvent& e)
    {
    }

    void itemDoubleClicked (const MouseEvent& e)
    {
    }

    void itemSelectionChanged (bool isNowSelected)
    {
        const String objectId (DrawableDocument::getIdFor (node));

        if (isNowSelected)
            editor.getSelection().addToSelection (objectId);
        else
            editor.getSelection().deselect (objectId);
    }

    void changeListenerCallback (void*)
    {
        setSelected (editor.getSelection().isSelected (DrawableDocument::getIdFor (node)), false);
    }

    const String getTooltip()
    {
        return String::empty;
    }

    const String getDragSourceDescription()
    {
        return drawableItemDragType;
    }

    //==============================================================================
    // Drag-and-drop stuff..
    bool isInterestedInFileDrag (const StringArray& files)
    {
        return false;
    }

    void filesDropped (const StringArray& files, int insertIndex)
    {
    }

    bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent)
    {
        return false;
    }

    void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex)
    {
    }

    //==============================================================================
    void showRenameBox()
    {
    }

    // Text editor listener for renaming..
    void textEditorTextChanged (TextEditor& textEditor)         {}
    void textEditorReturnKeyPressed (TextEditor& textEditor)    { textEditor.exitModalState (1); }
    void textEditorEscapeKeyPressed (TextEditor& textEditor)    { textEditor.exitModalState (0); }
    void textEditorFocusLost (TextEditor& textEditor)           { textEditor.exitModalState (0); }

    //==============================================================================
    DrawableEditor& editor;
    ValueTree node;

private:
    String typeName;

    DrawableEditor* getEditor() const
    {
        return getOwnerView()->findParentComponentOfClass ((DrawableEditor*) 0);
    }
};


#endif   // __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__

#ifndef ADLER_PROJECTDOCUMENT
#define ADLER_PROJECTDOCUMENT

#include "../includes.h"
#include "FilterGraph.h"
#include "GraphEditorPanel.h"
#include "ControlSurface.h"

class ProjectDocument : public FileBasedDocument
{
	ControlSurfaceComponent* controlSurfaceComp;
	FilterGraph* graph;
public:
	ProjectDocument(ControlSurfaceComponent*, FilterGraph*);

	//==============================================================================
    const String getDocumentTitle();
    const String loadDocument (const File& file);
    const String saveDocument (const File& file);
    const File getLastDocumentOpened();
    void setLastDocumentOpened (const File& file);
};

#endif

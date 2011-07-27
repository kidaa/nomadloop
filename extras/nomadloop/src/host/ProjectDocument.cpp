#include "ProjectDocument.h"

ProjectDocument::ProjectDocument(ControlSurfaceComponent* controlSurfaceComp, FilterGraph* graph)
	: FileBasedDocument(".nomad",
			"*.nomad",
			"Load a NomadLoop project",
			"Save a NomadLoop project"),
		controlSurfaceComp(controlSurfaceComp),
		graph(graph)
{
}

const String ProjectDocument::getDocumentTitle()
{
	if (! getFile().exists())
        return "Unnamed";

    return getFile().getFileNameWithoutExtension();
}

const String ProjectDocument::loadDocument (const File& file)
{
	XmlDocument doc (file);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName (T("NOMADPROJECT")))
    {
        delete xml;
        return "Not a valid NomadLoop project file";
    }

	forEachXmlChildElementWithTagName (*xml, e, T("FILTERGRAPH"))
	{
		graph->restoreFromXml (*e);
	}

	forEachXmlChildElementWithTagName (*xml, e, T("CONTROLSURFACE"))
	{
		controlSurfaceComp->restoreFromXml (*e);
	}

    delete xml;

    return String::empty;
}

const String ProjectDocument::saveDocument (const File& file)
{
	XmlElement* xml = new XmlElement(T("NOMADPROJECT")); //createXml();

	xml->addChildElement (graph->createXml());

	xml->addChildElement (controlSurfaceComp->createXml());

    String error;

    if (! xml->writeToFile (file, String::empty))
        error = "Couldn't write to the file";

    delete xml;
    return error;
}

const File ProjectDocument::getLastDocumentOpened()
{
	RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (appProperties->getUserSettings()
                                        ->getValue ("recentNomadProjectFiles"));

    return recentFiles.getFile (0);
}

void ProjectDocument::setLastDocumentOpened (const File& file)
{
	RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (appProperties->getUserSettings()
                                        ->getValue ("recentFilterGraphFiles"));

    recentFiles.addFile (file);

    appProperties->getUserSettings()
        ->setValue ("recentNomadProjectFiles", recentFiles.toString());
}

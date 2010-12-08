/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_EDITOR_MAP_CONTEXT_H
#define AX_EDITOR_MAP_CONTEXT_H

AX_BEGIN_NAMESPACE

class Layer {
public:
private:
	typedef DictSet<int> IdSet;
	int m_id;
	IdSet m_actors;

};

struct Bookmark 
{
	int id;
	std::string name;
	Matrix viewMatrix;
};

class AX_API MapContext : public Context {
public:
	enum ObserverFlag {
		SelectionChanged = 1,
		HistoryChanged = 2,
		StatusChanged = 4,
		TerrainMaterialEdited = 8,
		ToolChanged = 0x10,
		EnvironmentChanged = 0x20,
		ActorTransformed = 0x40,
		EverythingChanged = 0xffffffff
	};

	MapContext();
	~MapContext();

	void reset();

	std::string getTitle() const;
	std::string getFilename() const;
	bool createNew();
	bool load(const std::string &filename);
	bool save();
	bool saveAs(const std::string &filename);

	// view process
	void setActiveView(View *view) { m_activeView = view; }
	View *getActiveView() const { return m_activeView; }

	Vector3 getViewPos();

	MapTerrain *createTerrain(int tiles, int tilemeters);
	MapTerrain *getTerrain();
	void setTerrainMaterialDef(MapMaterialDef *matdef);

	GameWorld *getGameWorld() const { return m_gameWorld; }
	void runGame();

	// present
	virtual void doRender(const RenderCamera &camera, bool world = false);
	virtual void doHitTest(const RenderCamera &camera, int part);

	// bookmarks --timlly add
	void addBookmark(const Matrix &viewMatrix, const std::string &name = "", int id = -1);
	void addBookmark(const Bookmark &bookmark);
	void deleteBookmark(const std::string &name);
	void deleteBookmark(int index);

	int getNumBookmark();
	Bookmark *getBookmark(const std::string &name);
	Bookmark *getBookmark(int index);

	void clearAllBookmarks();

	// properties
	void setActorProperty(const std::string &propName, const Variant &value);

	// map state
	MapState *getMapState() const { return m_mapState; }
	void setMapState(MapState *val) { m_mapState = val; }

protected:
	void writeToFile(File *f);
	void readActor(const TiXmlElement *node);

	// save/load the helper info in the editor. --timlly add
	void saveEditorInfo(const std::string &filename);
	bool loadEditorInfo(const std::string &filename);

	// save/load the bookmark info. --timlly add
	void saveBookmarkInfo(File *file, int indent);
	void loadBookmarkInfo(const TiXmlElement *elem);

private:
	GameWorld *m_gameWorld;
	std::string m_title;
	std::string m_filename;
	MapTerrain *m_terrain;
	TerrainFixed *m_terrainFixed;

	// views
	PerspectiveView *m_perspectiveView;
	TopView *m_topView;
	FrontView *m_frontView;
	LeftView *m_leftView;

	// map state
	MapState *m_mapState;

	// bookmarks  --timlly add
	std::vector<Bookmark>	m_bookmarks;
	int m_bookmarkIndex;
};

inline std::string MapContext::getTitle() const {
	if (m_filename.empty())
		return m_title;
	return m_filename;
}

inline std::string MapContext::getFilename() const {
	return m_filename;
}

inline MapTerrain *MapContext::getTerrain() {
	return m_terrain;
}

AX_END_NAMESPACE

#endif // end guardian


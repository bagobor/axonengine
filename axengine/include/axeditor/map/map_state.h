/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_EDITOR_MAP_STATE_H
#define AX_EDITOR_MAP_STATE_H

AX_BEGIN_NAMESPACE

	class MapState : public State {
	public:
		MapState();

		float terrainBrushSize;
		float terrainBrushSoftness;
		float terrainBrushStrength;
		int terrainCurLayerId;

		bool followTerrain;
		std::string staticModelName;
		std::string treeFilename;
		std::string entityClass;		// current entity class for creation
	};

AX_END_NAMESPACE

#endif // AX_EDITOR_MAP_STATE_H
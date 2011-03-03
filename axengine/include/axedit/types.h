/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#ifndef AX_EDITOR_TYPES_H
#define AX_EDITOR_TYPES_H

#include <bitset>

AX_BEGIN_NAMESPACE

// some typedef

class Agent;
class AgentList;

class Action;
class Tool;

class Context;

class View;
class PerspectiveView;
class MapOverallView;

// cursor
struct CursorType {
	enum Type {
		Default,
		Arrow,			// standard arrow cursor 
		UpArrow,		// upwards arrow 
		Cross,			// crosshair 
		Wait,			// hourglass/watch 
		IBeam,			// ibeam/text entry 
		SizeVer,		// vertical resize 
		SizeHor,		// horizontal resize 
		SizeFDiag,		// diagonal resize () 
		SizeBDiag,		// diagonal resize (/) 
		SizeAll,		// all directions resize 
		Blank,			// blank/invisible cursor 
		SplitV,			// vertical splitting 
		SplitH,			// horizontal splitting 
		PointingHand,	// a pointing hand 
		Forbidden,		// a slashed circle 
		WhatsThis,		// an arrow with a question mark 
		Busy,			// standard arrow with hourglass/watch

		ViewRotate,
		ViewZoom,
		ViewPan,
	};
	AX_DECLARE_ENUM(CursorType);
};

struct SelectPart {
	enum Type {
		kTerrain = 1,
		kStatic = 2,
		kVegetation = 4,
		kActor = 8,

		All = 0xFFFFFFFF
	};
	AX_DECLARE_ENUM(SelectPart);
};

// clipboard
struct ClipBoardData {
	// clipboard
	enum Format {
		Empty, Text, MapActor, Actor,

		UserDefined = 128,

		MaxFormat = 256
	};

	Format format;
	size_t dataSize;
	void *data;
};

struct Space {
	enum Type {
		WorldSpace, ViewSpace, ObjectSpace
	};

	AX_DECLARE_ENUM(Space);
};

AX_END_NAMESPACE

#endif // AX_EDITOR_TYPES_H

#include "stdafx.h"

MPlug Global::getPlug( MPxNode* node, const char* longName )
{
	MObject nodeObj = node->thisMObject();
	MFnDependencyNode nodeFn(nodeObj);
	return nodeFn.findPlug(longName, true);
}

MColor Global::getColor( const MPlug& plug, MStatus* s /*= NULL*/ )
{
	MColor color;
	int nC = plug.numChildren(s);
	if (nC == 3)
	{
		float r = plug.child(0).asFloat();
		float g = plug.child(1).asFloat();
		float b = plug.child(2).asFloat();
		color = MColor(r,g,b);
	}
	else if (s)
		*s = MS::kFailure;
	return color;
}

void Global::displayError( const MPxNode* node, const MString& errorMsg )
{
	MGlobal::displayError(node->name() + ":\t" + errorMsg);
}

MStatus Global::getFloat( const MPlug& plug, float* buffer, int length)
{
	MStatus s;
	int nC = plug.numChildren(&s);
	if(nC != length)
		return MS::kFailure;
	for (int i = 0; i < nC; ++i)
	{
		buffer[i] = plug.child(i, &s).asFloat();
		CHECK_MSTATUS_AND_RETURN_IT(s)
	}
	return MS::kSuccess;
}

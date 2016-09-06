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

MStatus Global::getVDBData( MObject&matObj, OpenVDBData*& dat )
{
	//PRINT_F("get vdb data");
	MStatus s;
	MFnPluginData matFn(matObj, &s);

	CHECK_MSTATUS_AND_RETURN_IT(s);
	if (s && matFn.typeId() == OpenVDBData::id)
	{
		MPxData* pData= matFn.data(&s);
		if (pData && s)
		{
			dat = dynamic_cast<OpenVDBData*>(pData);
			//PRINT_F("pdata %p %p", pData, dat);
			if (dat)
				return s;
		}
	}
	dat = NULL;
	return s;
}

void Global::printMat( char* name, Matrix4f& mat )
{
	PRINT_F("%s:\n\t%lf\t%lf\t%lf\t%lf\n\t%lf\t%lf\t%lf\t%lf\n\t%lf\t%lf\t%lf\t%lf\n\t%lf\t%lf\t%lf\t%lf",name,
		mat(0,0), mat(0,1), mat(0,2), mat(0,3), 
		mat(1,0), mat(1,1), mat(1,2), mat(1,3), 
		mat(2,0), mat(2,1), mat(2,2), mat(2,3),
		mat(3,0), mat(3,1), mat(3,2), mat(3,3));
}

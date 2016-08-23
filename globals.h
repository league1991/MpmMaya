#pragma once

class Global
{
public:
	static MPlug getPlug(MPxNode* node, const char* longName);

	static MColor getColor(const MPlug& plug, MStatus* s = NULL);

	static void displayError(const MPxNode* node, const MString& errorMsg);
	static MStatus getFloat( const MPlug& plug, float* buffer, int length);
};
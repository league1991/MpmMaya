#include "stdafx.h"

const char* MpmSimulateCmd::s_cmdName = "mpmSimulate";

const char* MpmSimulateCmd::s_initFlag[2] = {"-init", "-i"};
const char* MpmSimulateCmd::s_stepFlag[2] = {"-step", "-s"};
const char* MpmSimulateCmd::m_nameFlag[2] = {"-name", "-n"};

MSyntax MpmSimulateCmd::newSyntax()
{
	MSyntax syntax;
	MStatus s;
	s = syntax.addFlag(s_initFlag[1], s_initFlag[0], MSyntax::kNoArg);
	s = syntax.addFlag(s_stepFlag[1], s_stepFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_recordEleGFFlag[1], m_recordEleGFFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_saveEleGFFlag[1], m_saveEleGFFlag[0], MSyntax::kString);
// 	s = syntax.addFlag(m_intPntFlag[1], m_intPntFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_surfPntFlag[1], m_surfPntFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_createFlag[1], m_createFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_stepStaticFlag[1], m_stepStaticFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_nameFlag[1], m_nameFlag[0], MSyntax::kString);
// 	s = syntax.addFlag(m_hessianFlag[1], m_hessianFlag[0], MSyntax::kDouble, MSyntax::kDouble);
// 	s = syntax.addFlag(m_gradFlag[1], m_gradFlag[0], MSyntax::kDouble, MSyntax::kDouble);
// 	s = syntax.addFlag(m_saveFlag[1], m_saveFlag[0], MSyntax::kString);
// 	s = syntax.addFlag(m_loadEleMatFlag[1], m_loadEleMatFlag[0], MSyntax::kNoArg);
// 	s = syntax.addFlag(m_resetEleMatFlag[1], m_resetEleMatFlag[0], MSyntax::kNoArg);
	return syntax;
}

void* MpmSimulateCmd::creator()
{
	return new MpmSimulateCmd;
}

MStatus MpmSimulateCmd::doIt( const MArgList& args )
{
	MStatus s; 

	MSelectionList selection;
	MSyntax syn = syntax();
	MArgDatabase argData(syn, args);
	setResult(0);
	if (argData.isFlagSet(m_nameFlag[1], &s))
	{
		MString nodeName;											// 提取某个名字的节点
		s = argData.getFlagArgument(m_nameFlag[1],0, nodeName);
		s = MGlobal::getSelectionListByName(nodeName, selection);
	}
	else
		s = MGlobal::getActiveSelectionList(selection);				// 提取选中节点
	if (!s)return MS::kFailure;

	bool isInitFlagSet = argData.isFlagSet(s_initFlag[1], &s);
	bool isStepFlagSet = argData.isFlagSet(s_stepFlag[1], &s);


	MItSelectionList pSel(selection, MFn::kDependencyNode , &s);
	MObject obj;
	for (; !pSel.isDone(); pSel.next())
	{
		MObject depNode;
		pSel.getDependNode(depNode);
		MFnDependencyNode nodeFn(depNode, &s);
		if (!s)
			continue;
		MString typeName = nodeFn.typeName(&s);
		if (!s)
			continue;

		// 找到模拟节点，执行之
		if (typeName == MpmSimulator::m_nodeName)
		{
			MpmSimulator* node = (MpmSimulator*)nodeFn.userNode(&s);
			if (!s || !node)
				continue;

			bool res = true;
			MString name = nodeFn.name(&s);
			if (isInitFlagSet)
			{
				res &= node->initSolver();
			}
			else if (isStepFlagSet)
			{
				res &= node->stepSolver();
			}

			if (res)
			{
				MString info = "rigSimulate command succeed in " + name;
				MGlobal::displayInfo(info);
			}
			else
			{
				MString info = "rigSimulate command failed in " + name;
				MGlobal::displayError(info);
				return MS::kSuccess;
			}
		}
	}
	setResult(1);
	return MS::kSuccess;
}

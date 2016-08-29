#include "float.h"

#include <vector>
#include <sstream>
#include <string>

#include <Eigen/SVD>
#include <Eigen/LU>
typedef Eigen::Matrix3f Matrix3f;
typedef Eigen::Vector3f	Vector3f;
typedef Eigen::Vector3i	Vector3i;

#include <tbb/mutex.h>

#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/integer_traits.hpp> // const_max
#include <boost/math/special_functions/fpclassify.hpp> // for isfinite
#include <boost/scoped_array.hpp>
#include <boost/multi_array.hpp>
#include <boost/array.hpp>

#include <openvdb/Platform.h>
#include <openvdb/openvdb.h>
#include <openvdb/Types.h> 

#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/MFloatVector.h>
#include <maya/MIOStream.h>
#include <maya/MPoint.h>
#include <maya/MSelectionList.h>
#include <maya/MMatrix.h>
#include <maya/MBoundingBox.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MDagPath.h>

#include <maya/MPxData.h>
#include <maya/MPxCommand.h>

#include <maya/MFnFluid.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNObjectData.h>
#include <maya/MnParticle.h>

#undef isnan

#include "OpenVDBData.h"
#include "MMPSolver_Apple.h"
#include "OpenVDBCopyNode.h"
#include "OpenVDBFilterNode.h"
#include "OpenVDBFromMayaFluidNode.h"
#include "OpenVDBFromPolygonsNode.h"
#include "OpenVDBReadNode.h"
#include "OpenVDBToPolygonsNode.h"
#include "OpenVDBTransformNode.h"
#include "OpenVDBVisualizeNode.h"
#include "OpenVDBWriteNode.h"

#include "globals.h"
#include "Particle.h"
#include "StatusRecorder.h"
#include "mpmCore.h"
#include "MpmSimulator.h"
#include "MpmSimulateCmd.h"
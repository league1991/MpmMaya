///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012-2015 DreamWorks Animation LLC
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// *     Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////

/// @author FX R&D OpenVDB team
#include "stdafx.h"
//#include "MMPSolver_Apple.h"
////////////////////////////////////////


namespace openvdb_maya {


namespace {

struct NodeInfo {
    MString typeName;
    MTypeId typeId;
    MCreatorFunction creatorFunction;
    MInitializeFunction initFunction;
    MPxNode::Type type;
    const MString* classification;
};

typedef std::vector<NodeInfo> NodeList;
typedef tbb::mutex Mutex;
typedef Mutex::scoped_lock Lock;

// Declare this at file scope to ensure thread-safe initialization.
Mutex sRegistryMutex;

NodeList * gNodes = NULL;

} // unnamed namespace


NodeRegistry::NodeRegistry(const MString& typeName, const MTypeId& typeId,
    MCreatorFunction creatorFunction, MInitializeFunction initFunction,
    MPxNode::Type type, const MString* classification)
{
    NodeInfo node;
    node.typeName           = typeName;
    node.typeId             = typeId;
    node.creatorFunction    = creatorFunction;
    node.initFunction       = initFunction;
    node.type               = type;
    node.classification     = classification;

    Lock lock(sRegistryMutex);

    if (!gNodes) {
        OPENVDB_START_THREADSAFE_STATIC_WRITE
        gNodes = new NodeList();
        OPENVDB_FINISH_THREADSAFE_STATIC_WRITE
    }

    gNodes->push_back(node);
}


void
NodeRegistry::registerNodes(MFnPlugin& plugin, MStatus& status)
{
    Lock lock(sRegistryMutex);

    if (gNodes) {
        for (size_t n = 0, N = gNodes->size(); n < N; ++n) {

            const NodeInfo& node = (*gNodes)[n];

            status = plugin.registerNode(node.typeName, node.typeId,
                node.creatorFunction, node.initFunction, node.type, node.classification);

            if (!status) {
                const std::string msg = "Failed to register '" +
                    std::string(node.typeName.asChar()) + "'";
                status.perror(msg.c_str());
                break;
            }
        }
    }
}


void
NodeRegistry::deregisterNodes(MFnPlugin& plugin, MStatus& status)
{
    Lock lock(sRegistryMutex);

    if (gNodes) {
        for (size_t n = 0, N = gNodes->size(); n < N; ++n) {

            const NodeInfo& node = (*gNodes)[n];

            status = plugin.deregisterData(node.typeId);

            if (!status) {
                const std::string msg = "Failed to deregister '" +
                    std::string(node.typeName.asChar()) + "'";
                status.perror(msg.c_str());
                break;
            }
        }
    }
}


} // namespace openvdb_maya


////////////////////////////////////////


MStatus
initializePlugin(MObject obj)
{
    openvdb::initialize();

    MStatus status;
    MFnPlugin plugin(obj, "DreamWorks Animation", "0.5", "Any");

    status = plugin.registerData("OpenVDBData", OpenVDBData::id, OpenVDBData::creator);
    if (!status) {
        status.perror("Failed to register 'OpenVDBData'");
        return status;

    }

	status = plugin.registerNode("OpenVDBTransform", OpenVDBTransformNode::id,OpenVDBTransformNode::creator, OpenVDBTransformNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBTransform'");
        return status;

    }
	status = plugin.registerNode("OpenVDBVisualize", OpenVDBVisualizeNode::id,OpenVDBVisualizeNode::creator, OpenVDBVisualizeNode::initialize, MPxNode::kLocatorNode);
	if (!status) {
        status.perror("Failed to register 'OpenVDBVisualize'");
        return status;

    }
	status = plugin.registerNode("OpenVDBWrite", OpenVDBWriteNode::id,OpenVDBWriteNode::creator, OpenVDBWriteNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBWrite'");
        return status;

    }
	status = plugin.registerNode("OpenVDBCopy", OpenVDBCopyNode::id,OpenVDBCopyNode::creator, OpenVDBCopyNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBCopy'");
        return status;

    }
	status = plugin.registerNode("OpenVDBFilter", OpenVDBFilterNode::id,OpenVDBFilterNode::creator, OpenVDBFilterNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBFilter'");
        return status;

    }
	status = plugin.registerNode("OpenVDBFromMayaFluid", OpenVDBFromMayaFluidNode::id,OpenVDBFromMayaFluidNode::creator, OpenVDBFromMayaFluidNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBFromMayaFluid'");
        return status;

    }
	status = plugin.registerNode("OpenVDBFromPolygons", OpenVDBFromPolygonsNode::id,OpenVDBFromPolygonsNode::creator, OpenVDBFromPolygonsNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBFromPolygons'");
        return status;

    }
	status = plugin.registerNode("OpenVDBRead", OpenVDBReadNode::id,OpenVDBReadNode::creator, OpenVDBReadNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBRead'");
        return status;

    }
	status = plugin.registerNode("OpenVDBToPolygons", OpenVDBToPolygonsNode::id,OpenVDBToPolygonsNode::creator, OpenVDBToPolygonsNode::initialize);
	if (!status) {
        status.perror("Failed to register 'OpenVDBToPolygons'");
        return status;

    }


	status = plugin.registerNode(MpmSimulator::m_nodeName, MpmSimulator::m_id,MpmSimulator::creator, MpmSimulator::initialize,MPxNode::kLocatorNode);
	if (!status) {
		status.perror("Failed to register 'MpmSimulator'");
		return status;
	}

	status = plugin.registerCommand(MpmSimulateCmd::s_cmdName, MpmSimulateCmd::creator, MpmSimulateCmd::newSyntax);
	if (!status) {
		status.perror("Failed to register 'mpmSimulateCmd'");
		return status;
	}

	glewInit();
    //openvdb_maya::NodeRegistry::registerNodes(plugin, status);

	


	MGlobal::displayInfo("Simulation plugin is loaded successfully!");
    return status;
}


MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterData(OpenVDBData::id);
    if (!status) {
        status.perror("Failed to deregister 'OpenVDBData'");
        return status;
    }
	plugin.deregisterNode(OpenVDBTransformNode::id);
	plugin.deregisterNode(OpenVDBVisualizeNode::id);
	plugin.deregisterNode(OpenVDBWriteNode::id);
	plugin.deregisterNode(OpenVDBCopyNode::id);
	plugin.deregisterNode(OpenVDBFilterNode::id);
	plugin.deregisterNode(OpenVDBFromMayaFluidNode::id);
	plugin.deregisterNode(OpenVDBFromPolygonsNode::id);
	plugin.deregisterNode(OpenVDBReadNode::id);
	plugin.deregisterNode(OpenVDBToPolygonsNode::id);
	plugin.deregisterNode(MpmSimulator::m_id);
	plugin.deregisterCommand(MpmSimulateCmd::s_cmdName);
    //openvdb_maya::NodeRegistry::deregisterNodes(plugin, status);

    return status;
}


////////////////////////////////////////


// Copyright (c) 2012-2015 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )

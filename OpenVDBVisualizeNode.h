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
#include "OpenVDBData.h"
#include "OpenVDBUtil.h"

#include <openvdb/io/Stream.h>

#include <maya/MPxLocatorNode.h>
#include <maya/M3dView.h>

////////////////////////////////////////

namespace mvdb = openvdb_maya;
struct OpenVDBVisualizeNode : public MPxLocatorNode
{
    OpenVDBVisualizeNode();
    virtual ~OpenVDBVisualizeNode();

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    virtual void draw(M3dView & view, const MDagPath & path,
        M3dView::DisplayStyle style, M3dView::DisplayStatus status);

    virtual bool isBounded() const;
    virtual MBoundingBox boundingBox() const;

    static void * creator();
    static  MStatus initialize();

    static MObject aVdbInput;
    static MObject aVdbAllGridNames;
    static MObject aVdbSelectedGridNames;

    static MObject aVisualizeBBox;
    static MObject aVisualizeInternalNodes;
    static MObject aVisualizeLeafNodes;
    static MObject aVisualizeActiveTiles;
    static MObject aVisualizeActiveVoxels;
    static MObject aVisualizeSurface;
    static MObject aIsovalue;

    static MObject aCachedBBox;
    static MObject aCachedInternalNodes;
    static MObject aCachedLeafNodes;
    static MObject aCachedActiveTiles;
    static MObject aCachedActiveVoxels;
    static MObject aCachedSurface;


    static MTypeId id;

private:
    std::vector<mvdb::BufferObject> mBBoxBuffers;
    std::vector<mvdb::BufferObject> mNodeBuffers;
    std::vector<mvdb::BufferObject> mLeafBuffers;
    std::vector<mvdb::BufferObject> mTileBuffers;
    std::vector<mvdb::BufferObject> mSurfaceBuffers;
    std::vector<mvdb::BufferObject> mPointBuffers;

    mvdb::ShaderProgram mSurfaceShader, mPointShader;
    MBoundingBox mBBox;
};



// Copyright (c) 2012-2015 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )

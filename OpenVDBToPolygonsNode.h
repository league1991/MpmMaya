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

/// @author Fredrik Salomonsson (fredriks@d2.com)
#include "OpenVDBData.h"
#include "OpenVDBUtil.h"

#include <openvdb/tools/VolumeToMesh.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>

#include <boost/scoped_array.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

////////////////////////////////////////


struct OpenVDBToPolygonsNode : public MPxNode
{
    OpenVDBToPolygonsNode() {};
    virtual ~OpenVDBToPolygonsNode() {};

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void * creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aVdbInput;
    static MObject aIsovalue;
    static MObject aAdaptivity;
    static MObject aVdbAllGridNames;
    static MObject aVdbSelectedGridNames;
    static MObject aMeshOutput;
};



// Copyright (c) 2012-2015 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )

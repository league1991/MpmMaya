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

#include <openvdb/tools/Dense.h>
#include <openvdb/math/Transform.h>

#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MFnFluid.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MBoundingBox.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxNode.h>


////////////////////////////////////////

struct OpenVDBFromMayaFluidNode : public MPxNode
{
public:
    OpenVDBFromMayaFluidNode() {}
    virtual ~OpenVDBFromMayaFluidNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aFluidNodeName;
    static MObject aVdbOutput;

    static MObject aDensity;
    static MObject aDensityName;
    static MObject aTemperature;
    static MObject aTemperatureName;
    static MObject aPressure;
    static MObject aPressureName;
    static MObject aFuel;
    static MObject aFuelName;
    static MObject aFalloff;
    static MObject aFalloffName;
    static MObject aVelocity;
    static MObject aVelocityName;
    static MObject aColors;
    static MObject aColorsName;
    static MObject aCoordinates;
    static MObject aCoordinatesName;
    static MObject aTime;
};



// Copyright (c) 2012-2015 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )

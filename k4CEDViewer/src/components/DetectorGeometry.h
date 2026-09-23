/*
 * Copyright (c) 2020-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DetectorGeometry_h
#define DetectorGeometry_h 1

#include <string>
#include <vector>

// --- DD4hep ---
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "DDRec/Surface.h"
#include "DDRec/SurfaceManager.h"

typedef std::vector<std::string> StringVec;

namespace k4ced {

// Set of geometric parameters for initialization of a CEDGeoBox class object
struct CEDGeoBox {
  double sizes[3];
  double center[3];
  double rotate[3];
};

// Set of geometric parameters for initialization of a CEDGeoTube class object
struct CEDGeoTubeParams {
  double Rmax;
  double Rmin;
  double inner_symmetry;
  double outer_symmetry;
  double phi0;
  double delta_phi;
  double delta_z;
  double z0;
  // boolean that decides if the GeoTube is drawn twice at two different zPositions
  bool isBarrel;
};

// Convenient summary of both parameter sets above as (tracker) layers may be drawn as one tube or as a sequence of
// staves (-->GeoBox)
struct LayerGeometry {
  CEDGeoTubeParams tube{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false};
  std::vector<CEDGeoBox> staves{};
};

/* Draws the detector geometry for CLIC and ILD.
 * features:
 * - improved, i.e. more exact, placements
 * - generic
 * - no GEAR dependence
 * - surface (optionally) drawn as set of lines
 *
 * author: Thorben Quast, CERN Summer Student 2015
 * date: 31/07/2015
 */
void drawDD4hepDetector(dd4hep::Detector& theDetector, bool _surfaces, StringVec _detailled);

// read out of the "_detailled" parameter
bool detailledDrawing(StringVec _detailled, std::string detName);

void getVisAttributes(dd4hep::DetElement det, unsigned& color, bool& visible);

// converts the parameters in LayeredCalorimeterData given by the appropriate drivers
// into those required by the CEDGeoTube
CEDGeoTubeParams CalorimeterParameterConversion(dd4hep::rec::LayeredCalorimeterData* calo);

// converts the parameters in ZDiskPetalsData given by the appropriate drivers
// into those required by the CEDGeoTube
CEDGeoTubeParams PetalParameterConversion(std::vector<dd4hep::rec::ZDiskPetalsData::LayerLayout>::iterator thisLayer);

// converts the parameters from a LayeredCalorimeterData layer given by the appropriate drivers
// into those required by the CEDGeoTube
CEDGeoTubeParams
CalorimeterLayerParameterConversion(std::vector<dd4hep::rec::LayeredCalorimeterData::Layer>::iterator thisLayer);

// converts the parameters from a FixedPadSizeTPCData given by the appropriate drivers
// into those required by the CEDGeoTube
CEDGeoTubeParams TPCParameterConversion(dd4hep::rec::FixedPadSizeTPCData* tpc);

// converts the parameters from a ZPlanarData::LayerLayout layer given by the appropriate drivers
// into those required by the CEDGeoBox (for drawing of staves) or by CEDGeoTube (for approximation of the set of
// staves into tubes)
LayerGeometry TrackerLayerParameterConversion(std::vector<dd4hep::rec::ZPlanarData::LayerLayout>::iterator thisLayer);

// draws the given surfaces as a set of individual lines
bool DrawSurfaces(const dd4hep::rec::SurfaceManager& surfMan, std::string detName, unsigned color, int layer);

} // namespace k4ced

#endif

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

#ifndef k4GaudiCEDUtils_h
#define k4GaudiCEDUtils_h 1

// --- DD4hep ---
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "DDRec/Vector3D.h"

#include "edm4hep/Track.h"

// workaround for Gaudi logging
// #define endmsg std::endl

namespace k4ced {
//  using endmsg = std::endl;

// workaround for Gaudi logging

typedef enum { Verbose = 1, Debug, Info, Warning, Error } LogLevel;

// in Gaudi:
//  namespace MSG {
//  enum Level { NIL = 0, VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL, ALWAYS, NUM_LEVELS };
//}

struct GlobalLog {

  static GlobalLog& instance() {
    static GlobalLog me;
    return me;
  }
  unsigned& level() { return _level; }

  std::string& name() { return _name; }

private:
  unsigned _level = 0;
  std::string _name = "unknown";
};

static std::ofstream null_stream("/dev/null");

std::ostream& verbose();
std::ostream& debug();
std::ostream& info();
std::ostream& warning();
std::ostream& error();

// structure for calculating the track length of given particles
struct CalorimeterDrawParams {
  double r_inner, delta_r, z_0, delta_z;
};

/**Helper struct for drawing collections*/
struct DrawParameters {
  DrawParameters(const std::string& colName, int size, int marker, int layer)
      : ColName(colName), Size(size), Marker(marker), Layer(layer) {}
  std::string ColName;
  int Size;
  int Marker;
  int Layer;
};

CalorimeterDrawParams getCalorimeterParameters(dd4hep::Detector& theDetector, std::string name, bool selfCall = false);

double calculateTrackLength(const CalorimeterDrawParams& barrel, const CalorimeterDrawParams& endcap, double x,
                            double y, double z, double px, double py, double pz, double rel_X0 = 0.5);

// get the outer extents of the tracker
std::array<double, 2> getTrackerExtent(dd4hep::Detector& theDetector);

// get the outer extents of the yoke
std::array<double, 2> getYokeExtent(dd4hep::Detector& theDetector);

// helper function to get track states
edm4hep::TrackState getTrackStateAt(const edm4hep::Track& trk, int location);

} // namespace k4ced

#endif

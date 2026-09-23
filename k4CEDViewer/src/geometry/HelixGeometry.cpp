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
#include "HelixGeometry.h"

#include "k4GaudiCEDUtils.h"

#include "ced_cli.h"

#include <cmath>

#define endmsg std::endl

namespace k4ced {

/**
 * Improved drawHelix() method. Draws straight lines as well.
 */
// SM-H: Added id to drawHelix (default zero), which allows for implementation of picking
void drawHelix(float b, float charge, float x, float y, float z, float px, float py, float pz, int marker, int size,
               unsigned int col, float rmin, float rmax, float zmax, unsigned int id) {
  // FIXME : check for zmin as well, i.e. cylindrical coordinates

  double cFactor = 2.9979251e-4;
  const double high_pt = 100.0; // Transverse momentum high enough for the particle not to curve noticeably
  double pt = sqrt(px * px + py * py);

  // FIXME: use a parameter for this cut or better this should be a function of the B field, charge and momentum
  // 2006/07/04 OW

  // SD: FIXME: Adaptive step-number (or get rid of it!) and adaptive draw step!
  if ((pt >= 0.01) && (pt <= high_pt && charge != 0)) {
    double r = pt / (cFactor * b * std::abs(charge));
    double sign = charge > 0 ? 1 : -1;

    sign = -sign; // FIXME: need to check the convention - but this works !?

    double phi = std::atan2(py, px) + (2. + sign) * M_PI / 2.;
    // center of helix
    double cx = x - (sign * py * r / pt);
    double cy = y + (sign * px * r / pt);
    double cz = z;

    double x1 = x;
    double y1 = y;
    double z1 = z;
    double step = 0.05; // initial 0.05

    // FIX ME: do the adaptive step number...

    // cheap adaptive algorithms
    if (px > 1 || py > 1 || px < -1 || py < -1) {
      step = 0.005;
      if (px > 5 || py > 5 || px < -5 || py < -5) {
        step = 0.001;
      }
    }

    int nSteps = int(100 / step); // hauke

    int count_lines = 0;
    for (int j = 0; j < nSteps; j++) {

      double alpha0 = step * j;

      double x2 = cx + r * cos(phi + sign * alpha0);
      double y2 = cy + r * sin(phi + sign * alpha0);
      double z2 = cz + r * alpha0 * pz / pt;

      double r_current = sqrt(x2 * x2 + y2 * y2); // hypot( x2, y2 )

      /*
       *  interpolation and loop break
       */
      if (std::abs(z2) > zmax || r_current > rmax) {

        double alpha = step * (j + 0.5);

        x2 = cx + r * cos(phi + sign * alpha);
        y2 = cy + r * sin(phi + sign * alpha);
        z2 = cz + r * alpha * pz / pt;
        break;
      }

      if (r_current >= (rmin + step)) {
        count_lines++;
        ced_line_ID(x1, y1, z1, x2, y2, z2, marker, size, col, id);
      }
      x1 = x2;
      y1 = y2;
      z1 = z2;
    }
  }
  // For high momentum tracks, just draw straight line
  else if (pt > high_pt) {
    debug() << "pt = " << pt << endmsg;
    float absP = sqrt(px * px + py * py + pz * pz);
    float k = 0.0;
    float kr = 0.0;
    float kz = 0.0;
    float summand = 0.0;
    float radicant = 0.0;

    // find intersection with rmax
    summand = (-1) * (absP * (px * x + py * y) / (pow(px, 2) + pow(py, 2)));
    radicant =
        summand * summand - ((pow(absP, 2) * (pow(x, 2) + pow(y, 2) - pow(rmax, 2))) / (pow(px, 2) + pow(py, 2)));

    if (radicant < 0) {
      error() << "Error in 'drawHelix()': Startpoint beyond (rmax,zmax)" << endmsg;
      return;
    }

    kr = summand + sqrt(radicant);
    kz = ((zmax - z) * absP) / pz;

    // this has been improved

    if (z + (kr * pz) / absP > zmax || z + (kr * pz) / absP < -zmax) {
      k = kz;
    } else
      k = kr;

    if (k < 0.0) {
      debug() << "drawHelix(): negative intersection parameter - will revert sign ... " << endmsg;
      // fg: k cannot be negativ ( particle is moving along its 3-momentum ....)
      k = -k;
    }

    float xEnd = x + (k * px) / absP;
    float yEnd = y + (k * py) / absP;
    float zEnd = z + (k * pz) / absP;

    if (rmin != 0) {
      debug() << "FIX ME: Inner cylinder not taken into account!" << endmsg;
      return;
    }

    debug() << "drawHelix()' - pt : " << pt << " |p| = " << absP << ", x " << x << ", y " << y << ", z " << z
            << ", px " << px << ", py " << py << ", pz " << pz << ", xEnd " << xEnd << ", yEnd " << yEnd << ", zEnd "
            << zEnd << endmsg;

    ced_line_ID(x, y, z, xEnd, yEnd, zEnd, marker, size, col, id);

  } else {
    debug() << "Low momentum particle given point instead of helix" << endmsg;
    const double delta = 0.0001;
    ced_line_ID(x, y, z, x + delta, y + delta, z + delta, marker, size, col, id);
  }
}

void DDdraw_helix(float b, float charge, float x, float y, float z, float px, float py, float pz, int marker, int size,
                  unsigned int col, float rmin, float rmax, float zmax, unsigned int id) {

  drawHelix(b, charge, x, y, z, px, py, pz, marker, size, col, rmin, rmax, zmax, id);
}

} // namespace k4ced

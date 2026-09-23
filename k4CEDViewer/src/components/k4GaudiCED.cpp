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
#include "k4GaudiCED.h"

#include "k4GaudiCEDUtils.h"

// #include <LCGeometryTypes.h>
#include "ced_cli.h"

#include <signal.h>

#include <unistd.h> //

// SJA:FIXED:added to make gcc4.3 compliant
#include <cstdio>
#include <cstdlib>

// hauke
#include <ctime>
#include <poll.h>
#include <termios.h>
#include <time.h>

// for kbhit
#include <sys/select.h>
#include <termios.h>

#define endmsg std::endl

namespace k4ced {

k4GaudiCED* k4GaudiCED::_me = 0;

std::vector<std::string> k4GaudiCED::_descs(CED_MAX_LAYER, ""); // layer descriptions

//--------------------------------------------------------------------------------------------------------

int PickingHandler::kbhit(void) {
  // http://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input#448982
  struct timeval tv = {0L, 0L};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}

void k4GaudiCED::add_layer_description(const std::string& desc, int layerID) {
  std::string tmp;
  if (layerID > CED_MAX_LAYER || layerID < 0) {
    return;
  }
  if (_descs.at(layerID).find(desc.c_str()) == std::string::npos) {
    tmp = _descs.at(layerID);
    if (!tmp.empty()) {
      tmp.append(", ");
    }
    tmp.append(desc);
    _descs.at(layerID) = tmp;
  } else {
  }
}

void k4GaudiCED::set_layer_description(const std::string& desc, int layerID) {
  if (layerID > CED_MAX_LAYER || layerID < 0) {
    return;
  }
  _descs.at(layerID) = desc;
}

void k4GaudiCED::write_layer_description(void) {
  // std::cout<<"LAYER: write all layer in ced" << std::endl;
  unsigned int i;
  // for(i=0;i<25;i++){
  for (i = 0; i < _descs.size(); i++) {
    ced_describe_layer(_descs.at(i).c_str(), i);
  }
}

k4GaudiCED* k4GaudiCED::instance() {
  if (_me == 0)
    _me = new k4GaudiCED;
  return _me;
}

void k4GaudiCED::init(const void* proc) {

  if (instance()->_first == 0) {

    instance()->_first = proc;

    char *port, *host;
    port = getenv("CED_PORT");
    host = getenv("CED_HOST");
    if ((port == NULL || port[0] == 0) && (host == NULL || host[0] == 0)) {
      ced_client_init("localhost", 7286);
    } else if (port == NULL || port[0] == 0) {
      info() << "Use user defined host " << host << endmsg;
      ced_client_init(host, 7286);
    } else if (host == NULL || host[0] == 0) {
      info() << "Use user defined port " << port << endmsg;
      ced_client_init("localhost", atoi(port));
    } else {
      info() << "Use user defined host " << host << ", port " << port << endmsg;
      ced_client_init(host, atoi(port));
    }

    ced_register_elements();
  }

  instance()->_last = proc;
}

void k4GaudiCED::newEvent(const void* proc) {
  if (proc == instance()->_first) {
    ced_new_event();

    PickingHandler::instance().clear();
  }
}

// hauke hoelbe modify 08.02.2010
void k4GaudiCED::draw(const void* proc, int waitForKeyboard) {
  int i = 0;

  if (proc == instance()->_last) {
    //    ced_draw_event();
    k4GaudiCED::write_layer_description();
    // ced_picking_text("test1 test2 test3");

    ced_send_event();
    if (waitForKeyboard == 1) {
      info() << "Double click for picking. Press <ENTER> for the next event." << endmsg;
      // test:

      signal(SIGWINCH, SIG_IGN);

      while (!PickingHandler::kbhit()) {
        //            while(!poll(pfd,1,0)){

        usleep(100000); // micro seconds

        int id = ced_selected_id_noblock();

        if (id >= 0) {

          debug() << "debug: got id: " << id << endmsg;

          if (id == 0) {

            warning() << "Picking nothing, or an object with ID 0!" << endmsg;

          } else {

            PickingHandler::instance().printObject(id);

            ced_picking_text("test1 test2 test3", i++);
            ced_send_event();
          }
        }
      }

      signal(SIGWINCH, SIG_IGN);
      int c = getchar();
      if (c == 'q' || c == 'Q' || c == 3) { // quit if the user pressed q or strg+c (3 = strg+c)
        exit(0);
      }
      info() << "--------- END ---------------\n";
    }
  }
}

/**
 * Improved drawHelix() method. Draws straight lines as well.
 */
// SM-H: Added id to drawHelix (default zero), which allows for implementation of picking
void k4GaudiCED::drawHelix(float b, float charge, float x, float y, float z, float px, float py, float pz, int marker,
                           int size, unsigned int col, float rmin, float rmax, float zmax, unsigned int id) {
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
      error() << "Error in 'k4GaudiCED::drawHelix()': Startpoint beyond (rmax,zmax)" << endmsg;
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
      debug() << "k4GaudiCED::drawHelix(): negative intersection parameter - will revert sign ... " << endmsg;
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

    debug() << "k4GaudiCED::drawHelix()' - pt : " << pt << " |p| = " << absP << ", x " << x << ", y " << y << ", z "
            << z << ", px " << px << ", py " << py << ", pz " << pz << ", xEnd " << xEnd << ", yEnd " << yEnd
            << ", zEnd " << zEnd << endmsg;

    ced_line_ID(x, y, z, xEnd, yEnd, zEnd, marker, size, col, id);

  } else {
    debug() << "Low momentum particle given point instead of helix" << endmsg;
    const double delta = 0.0001;
    ced_line_ID(x, y, z, x + delta, y + delta, z + delta, marker, size, col, id);
  }
}

void DDdraw_helix(float b, float charge, float x, float y, float z, float px, float py, float pz, int marker, int size,
                  unsigned int col, float rmin, float rmax, float zmax, unsigned int id) {

  k4GaudiCED::drawHelix(b, charge, x, y, z, px, py, pz, marker, size, col, rmin, rmax, zmax, id);
}

} // namespace k4ced

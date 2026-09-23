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

} // namespace k4ced

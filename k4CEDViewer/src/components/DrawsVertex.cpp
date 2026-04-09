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

#include "edm4hep/VertexCollection.h"
#include "k4FWCore/Consumer.h"
#include "k4GaudiCED.h"

#include "k4CEDColors.h"
#include "k4GaudiCEDUtils.h"

#include <string>

using namespace k4ced;

struct DrawVertices final : k4FWCore::Consumer<void(const std::vector<const edm4hep::VertexCollection*>&)> {
  DrawVertices(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValues("colName", {"Vertices"})) {
    k4GaudiCED::init(this);
  }

  Gaudi::Property<int> layer{this, "layer", 6, "layer to draw Vertices"};

  Gaudi::Property<int> size{this, "size", 2, "size for drawing Vertices"};

  Gaudi::Property<int> marker{this, "marker", 0, "marker for drawing Vertices"};

  Gaudi::Property<unsigned> colorScheme{this, "colorScheme", 12,
                                        "Red:0,Orange:1,Plum:2,Violet:3,Blue:4,LightBlue:5,Aquamarine:6,Green:7,Olive:"
                                        "8,Yellow:9,Dark:10,Light:11,Classic:12"};

  //===========================================================================================

  void operator()(const std::vector<const edm4hep::VertexCollection*>& collections) const override {

    k4ced::GlobalLog::instance().level() = msgSvc()->outputLevel();
    k4ced::GlobalLog::instance().name() = name();

    k4GaudiCED::newEvent(this);

    std::stringstream sstr;
    info() << " +++++++  drawing Vertices from collections: \n";
    for (unsigned i = 0; i < inputLocations(0).size(); ++i) {

      sstr << inputLocations(0)[i] << ", ";
      info() << "    " << inputLocations(0)[i] << "\n";
    }
    info() << endmsg;

    Colors colors(colorScheme);

    //-----------------------

    k4GaudiCED::add_layer_description(sstr.str(), layer);

    unsigned ic = 0;
    for (const auto* col : collections) {
      unsigned myColID = PickingHandler::instance().colID() * k4ced::IDFactor;
      printfun f = PrintEDM4hep<edm4hep::VertexCollection>(*col);
      PickingHandler::instance().registerFunctor(myColID / IDFactor, f);

      // Assign color per collection
      unsigned color = colors.current()[ic++ % colors.size()];

      for (auto h : *col) {
        ced_hit_ID(h.getPosition()[0], h.getPosition()[1], h.getPosition()[2], marker, layer, size, color,
                   myColID + h.id().index);
      }
    }

    k4GaudiCED::draw(this, 1);
  }
};

DECLARE_COMPONENT(DrawVertices);

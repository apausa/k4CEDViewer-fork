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

#include "ColorMap.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "k4CEDColors.h"
#include "k4FWCore/Consumer.h"
#include "k4GaudiCED.h"
#include "k4GaudiCEDUtils.h"

#include "DD4hep/DetType.h"

#include "TVector3.h"

#include <string>

using namespace k4ced;

struct DrawJets final : k4FWCore::Consumer<void(const edm4hep::ReconstructedParticleCollection&)> {
  DrawJets(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValue("colName", {"Jets"})) {

    k4GaudiCED::init(this);
  }

  Gaudi::Property<int> layer{this, "layer", 4, "layer to draw ReconstructedParticles "};
  Gaudi::Property<std::string> m_ecalBarrelName{this, "EcalBarrelName", "EcalBarrel", "name of ecal barrel detector "};
  Gaudi::Property<std::string> m_ecalEndcapName{this, "EcalEndcapName", "EcalEndcap", "name of ecal endcap detector "};
  // Gaudi::Property<int> size{this, "size", 2, "size for drawning  ReconstructedParticles "};
  // Gaudi::Property<int> marker{this, "marker", 0, "marker for drawning  ReconstructedParticles "};

  
  //===========================================================================================

  void operator()(const edm4hep::ReconstructedParticleCollection& col) const override {

    k4ced::GlobalLog::instance().level() = msgSvc()->outputLevel();
    k4ced::GlobalLog::instance().name() = name();

    k4GaudiCED::newEvent(this);

    info() << " +++++++  drawing Jets (ReconstructedParticle) collection with " << col.size() << " particles "
           << " outputLevel = " << k4ced::GlobalLog::instance().level() << endmsg;

    unsigned myColID = PickingHandler::instance().colID() * k4ced::IDFactor;

    printfun f = PrintEDM4hep<edm4hep::ReconstructedParticleCollection>(col);
    PickingHandler::instance().registerFunctor(myColID / IDFactor, f);

    dd4hep::Detector& theDetector = dd4hep::Detector::getInstance();


//    Colors colors(colorScheme);

    //default color is orange
    float RGBAcolor[4] = {.9, .7, .0, 0.25};
    int color = int(RGBAcolor[2]*(15*16+15)) + int(RGBAcolor[1]*(15*16+15))*16*16+ int(RGBAcolor[0]*(15*16+15))*16*16*16*16;
    
    //only one registration for all jets in CEDViewer
    k4GaudiCED::add_layer_description( inputLocations(0)[0], layer);

    CalorimeterDrawParams ecalBarrelParams = getCalorimeterParameters(theDetector, m_ecalBarrelName);
    CalorimeterDrawParams ecalEndcapParams = getCalorimeterParameters(theDetector, m_ecalEndcapName);

    
    //------------------------
    for ( auto jet : col ){

      debug() <<   " - jet energy " << jet.getEnergy() << std::endl ;

      //total momentum of the jet
      TVector3 v(jet.getMomentum()[0], jet.getMomentum()[1], jet.getMomentum()[2]);

      //init relevant objects for looping over all particles in the jet
      auto pv = jet.getParticles();

      int N_elements = pv.size();
      float pt_tot = 0.0; float E_max = 0.0; float mean_tan_angle = 0.0;
      std::vector<TVector3> pp; std::vector<float> pt; std::vector<float> E;
      pp.reserve(N_elements); pt.reserve(N_elements); E.reserve(N_elements);

      //calculate longitudinal, transverse momentum (w.r. to jet axis) for each particle
      //from that deduce a pt-weighted mean (tan-) angle and determine the highest pt contribution
//      for (int k = 0; k<N_elements; ++k){
      unsigned kk=0 ;
      for( auto p : pv){
	TVector3 pp_k(p.getMomentum()[0], p.getMomentum()[1], p.getMomentum()[2]);
	pp.push_back(pp_k);
	TVector3 ju = v.Unit();
	TVector3 pt_k = pp_k - (ju.Dot(pp_k))*ju;
	TVector3 pl_k = pp_k - pt_k;
	pt.push_back(pt_k.Mag());
	pt_tot += pt[kk];
	E.push_back(p.getEnergy());
	E_max = (E[kk] > E_max) ? E[kk]: E_max;
	mean_tan_angle += pt[kk]*(pt[kk]/pl_k.Mag());
	++kk;
      }
      mean_tan_angle /= pt_tot;

      //draw the line of movement for each particle in the jet
      for (int k = 0; k<N_elements; ++k){
	float center_ref[3] = {0., 0., 0.};
	//100% * distance = length holds for the entry with highest pt, the others obtain only a respective fraction
	double momLength = (E[k]/E_max)*calculateTrackLength(ecalBarrelParams,ecalEndcapParams, center_ref[0], center_ref[1], center_ref[2], pp[k].X(), pp[k].Y(), pp[k].Z(), 0);                    //line size
	//approximation: all lines start in origin (TODO, if jet origin known)
	ced_line_ID(center_ref[0], center_ref[1], center_ref[2], momLength*pp[k].X()/pp[k].Mag(), momLength*pp[k].Y()/pp[k].Mag(), momLength*pp[k].Z()/pp[k].Mag(), layer, 1,
		    color, pv[k].id().index  );
      }

      //calculate the parameters of the jet cone
      //approximation: all lines start in origin (TODO, if jet origin known)
      double center_c[3] = {0., 0., 0. };
      double rotation_c[3] = { 0.,  v.Theta()*180./M_PI , v.Phi()*180./M_PI };
      double coneHeight = calculateTrackLength(ecalBarrelParams,ecalEndcapParams, center_c[0], center_c[1], center_c[2], v.X(), v.Y(), v.Z(), 0);
      //1. baseline radius, 2. height, 3. origin doublet, 4. rotation triplet,...
      ced_cone_r_ID( mean_tan_angle * coneHeight , coneHeight , center_c, rotation_c, layer, RGBAcolor, myColID +  jet.id().index  );

    }
 

    k4GaudiCED::draw(this, 1);
  }

};

DECLARE_COMPONENT(DrawJets)

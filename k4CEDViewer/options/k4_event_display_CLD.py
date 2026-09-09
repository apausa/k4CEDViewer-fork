#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from Gaudi.Configuration import (DEBUG, INFO, WARNING)
from k4FWCore import ApplicationMgr
from k4FWCore.parseArgs import parser
from k4FWCore import IOSvc

from Configurables import (
  GeoSvc,
  DrawDetector,
  DrawTracks,
  DrawClusters,
  DrawReconstructedParticles,
  DrawJets,
  DrawSimTrackerHits,
  DrawTrackerHits,
  DrawSimCalorimeterHits,
  DrawCalorimeterHits,
  DrawVertices,
  EventDataSvc
)

algList = []
svcList = []

parser.add_argument(
    "--compactFile", help="Compact detector file to use", type=str, default=""
)

reco_args = parser.parse_known_args()[0]
compact_file = reco_args.compactFile

evtsvc = EventDataSvc("EventDataSvc")
svcList.append(evtsvc)

iosvc = IOSvc("IOSvc")
iosvc.Input = "input_edm4hep.root"
#svcList.append(iosvc)

geoSvc = GeoSvc("GeoSvc")
geoSvc.detectors = [compact_file]
geoSvc.OutputLevel = INFO
geoSvc.EnableGeant4Geo = False
svcList.append(geoSvc)

algList.append(DrawDetector("draw_detector", drawSurfaces=False) )

# edm4hep::SimTrackerHitCollection
algList.append(DrawSimTrackerHits(
    "draw_simths",
    colNames = [
      "InnerTrackerBarrelCollection",
      "InnerTrackerEndcapCollection",
      "OuterTrackerBarrelCollection",
      "OuterTrackerEndcapCollection",
      "VertexBarrelCollection",
      "VertexEndcapCollection",
    ],
    layer=1,
    size=2
))

# edm4hep::SimCalorimeterHitCollection
algList.append(DrawSimCalorimeterHits(
  "draw_simchs",
  colNames = [
    "ECalBarrelCollection",
    "ECalEndcapCollection",
    "HCalBarrelCollection",
    "HCalEndcapCollection",
    "HCalRingCollection",
    "LumiCalCollection",
    "YokeBarrelCollection",
    "YokeEndcapCollection",
  ],
  layer=2,
  size=2)
)

# edm4hep::ReconstructedParticleCollection (jets)
# algList.append(DrawJets("draw_jets", colName="Jet", layer=4))

# edm4hep::TrackCollection
# algList.append(DrawTracks(
#  "draw_trks",
#  colName="EFlowTrack",
#  marker=0,
#  layer=5,
#  size=3,
#  drawHelixForTracks=0,
#  colorScheme=7
# ))

# edm4hep::VertexCollection
algList.append(DrawVertices(
  "draw_vtx",
  colName = [
    "BuildUpVertices",
    "BuildUpVertices_V0",
    "PandoraStartVertices",
    "PrimaryVertices",
    "RefinedVertexJets_vtx",
    "RefinedVertices",
  ],
  layer=6,
  size=100, # To differentiate vertices from calorimeter and tracker hits
  marker=3 # Marker 1 has a 'x' shape, marker 2 has an '+' shape, marker 3 has an '*' shape.
))

# edm4hep::ClusterCollection
# algList.append(DrawClusters("draw_clus_neutralhadron", colName="EFlowNeutralHadron", layer=8))
# algList.append(DrawClusters("draw_clus_photon", colName="EFlowPhoton", layer=8))

# edm4hep::ReconstructedParticleCollection
# algList.append(DrawReconstructedParticles("draw_pfos_electron", colName="Electron", drawHelixForPFOs=0))
# algList.append(DrawReconstructedParticles("draw_pfos_muon", colName="Muon", drawHelixForPFOs=0))
# algList.append(DrawReconstructedParticles("draw_pfos_photon", colName="Photon", drawHelixForPFOs=0))
# algList.append(DrawReconstructedParticles("draw_pfos_reco", colName="ReconstructedParticles", drawHelixForPFOs=0))

# edm4hep::TrackerHit3DCollection
# algList.append(DrawTrackerHits(
#  "draw_ths",
#  colNamesTH3D = ["TrackerHits"],
#  colNamesTHPlane = [],
#  layer=11,
#  size=4
# ))

# edm4hep::CalorimeterHitCollection
# algList.append(DrawCalorimeterHits("draw_chs", colNames = ["CalorimeterHits"], layer=12, size=4) )

ApplicationMgr(
  TopAlg=algList,
  EvtSel="NONE",
  EvtMax=100,
  ExtSvc=svcList,
  OutputLevel=WARNING,
)

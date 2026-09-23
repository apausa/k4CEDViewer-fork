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

#ifndef HelixGeometry_h
#define HelixGeometry_h 1

namespace k4ced {

/** Draws a helix from the given point(x,y,z) for momentum(px,py,pz) in a B-field b (in Tesla)
 */
void drawHelix(float b, float charge, float x, float y, float z, float px, float py, float pz, int marker, int size,
               unsigned int col, float rmin = 10.0, float rmax = 3000.0, float zmax = 4500.0, unsigned int id = 0);

extern "C" void DDdraw_helix(float b, float charge, float x, float y, float z, float px, float py, float pz,
                             int marker, int size, unsigned int col, float rmin = 10.0, float rmax = 3000.0,
                             float zmax = 4500.0, unsigned int id = 0);

} // namespace k4ced

#endif

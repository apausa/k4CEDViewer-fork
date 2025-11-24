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
#ifndef COLORMAP_H_
#define COLORMAP_H_

/**
 * This class has been obtained from
 * http://www.koders.com/cpp/fidCFEC52F6E8D5CAF77FBCDB42FDB45A69EC48677B.aspx?s=colorMapFunc#L153
 * under the GNU licence.
 * Minor modifications by:
 * @author: S.Daraszewicz
 * @date: 28.08.08
 */

namespace k4ced {

typedef void (*colorMapFunc)(unsigned int*, float, float, float);

typedef struct RgbColor {
  double r;
  double g;
  double b;
} RgbColor;

typedef struct HsvColor {
  double h;
  double s;
  double v;
} HsvColor;

class ColorMap {
public:
  static void colorMap(unsigned int* rgb, float value, float min, float max);
  static void hotColorMap(unsigned int* rgb, float value, float min, float max);
  static void coldColorMap(unsigned int* rgb, float value, float min, float max);
  static void jetColorMap(unsigned int* rgb, float value, float min, float max);
  static void cyclicColorMap(unsigned int* rgb, float value, float min, float max);
  static void randColorMap(unsigned int* rgb, float value, float min, float max);
  static void grayColorMap(unsigned int* rgb, float value, float min, float max);
  static void blueColorMap(unsigned int* rgb, float value, float min, float max);
  static colorMapFunc selectColorMap(int cmp);
  static int RGB2HEX(int red, int green, int blue);
  static RgbColor HsvToRgb(HsvColor in);
  static unsigned long NumberToTemperature(double value, double min, double max, double s, double v);
};

} // namespace k4ced
#endif

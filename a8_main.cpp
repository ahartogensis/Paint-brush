/* --------------------------------------------------------------------------
 * File:    a12_main.cpp
 * Created: 2021-10-28
 * --------------------------------------------------------------------------
 *
 *
 *
 * ------------------------------------------------------------------------*/

#include "Image.h"
#include "basicImageManipulation.h"
#include "npr.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

void brushtest(){
  Image castle("./input/castle.png");
  Image im = castle; 
  Image brush_stroke("./input/brush.png");
  vector<float> color = {0.5, 0.5, 0.5};
  for(int i = 0; i < 10; i++){
    int x = rand() % im.width(); 
    int y = rand() % im.height(); 
    brush(im, x, y, color, brush_stroke); 
  }
  im.write("./output/brush_test.png");
}

void test_singleScalePaint(){
  Image castle("./input/castle.png");
  Image im = castle; 
  Image china("./input/china.png");
  Image brush_stroke("./input/brush.png");
  singleScalePaint(china, im, brush_stroke);
  im.write("./output/singleScarePaint_test.png");
}


// void test_importance(){
//   Image castle("./input/castle.png");
//   Image im = castle; 
//   Image china("./input/china.png");
//   Image brush_stroke("./input/brush.png");
//   Image 
//   singleScalePaintImportance(china, im, brush_stroke);
//   im.write("./output/singleScarePaint_test.png");
// }

void test_sharpnessMap(){
  Image castle("./input/castle.png");
  Image sharpness = sharpnessMap(castle, 1.0); 
  sharpness.write("./output/sharpnessMap_test.png");
}

void test_paiter(){
  Image castle("./input/castle.png");
  Image im = castle; 
  Image china("./input/china.png");
  Image brush_stroke("./input/brush.png");
  painterly(china, im, brush_stroke); 
  im.write("./output/test_painterly.png");
}

void angle_test(){
  Image round("./input/round.png");
  Image im = testAngle(round); 
  im.write("./output/angle_test.png");
}

void orientedPaint_test(){
  Image china("./input/china.png");
  Image im = china;
  Image brush_stroke("./input/brush.png");

  orientedPaint(china, im, brush_stroke, 10000, 10);
  im.write("./output/orientedPaint_test.png");
}

void my_image(){
  Image dog("./input/my_image.png");
  Image im = dog; 
  Image brush_stroke("./input/brush.png");

  orientedPaint(dog, im, brush_stroke, 10000, 10);
  im.write("./input/my_image_oriented.png");
}

// This is a way for you to test your functions.
// We will only grade the contents of npr.cpp
int main() {
  cout << "Tests !" << endl;
  // brushtest(); 
  // test_singleScalePaint(); 
  // test_sharpnessMap(); 
  // test_paiter(); 
  // angle_test();
  orientedPaint_test();
  // my_image();
  
  return 0;
}

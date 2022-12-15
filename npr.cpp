// npr.cpp
// Assignment 8

#include "npr.h"
#include "filtering.h"
#include "matrix.h"
#include <algorithm>
#include <math.h>


using namespace std;

/**************************************************************
 //                       NPR                                //
 *************************************************************/

void brush(Image &im, int x, int y, vector<float> color, const Image &texture) {
	// Draws a brushstroke defined by texture and color at (x,y) in im
	// // --------- HANDOUT  PS12 ------------------------------
	if(x > texture.width()/2 && x < im.width() - texture.width()/2 
		&& y > texture.height()/2 && y < im.height() - texture.height()/2){
			for(int i = x - texture.width()/2; i < x + texture.width()/2; i++){
				for(int j = y - texture.height()/2; j < y + texture.height()/2; j++){
					for(int c = 0; c < color.size(); c++){
						int texture_x = i - (x - texture.width()/2); 
						int texture_y = j - (y - texture.height()/2);
						im(i,j,c) = im(i,j,c) * (1 - texture(texture_x, texture_y, c)) + color[c]*texture(texture_x,texture_y,c); 
					}
				}
			}
		}
	return;
}

void singleScalePaint(const Image &im, Image &out, const Image &texture, int size, int N, float noise) {
	// Create painted rendering by splatting brushstrokes at N random locations
	// in your ouptut image
	// // --------- HANDOUT  PS12 ------------------------------

	//scale the brushstroke to have max size 
	float scale = (float) size/ max(texture.height(), texture.width()); 
	Image brush_stroke = scaleLin(texture, scale);

	//Then we will splat a brushstroke on N random locations 
	for(int i = 0; i < N; i++){
		//make sure that the random location is within the image
		int x = rand() % im.width(); 
    	int y = rand() % im.height(); 
		vector<float> color(im.channels(), 0.0); 
		for(int z = 0; z < im.channels(); z++){
			//divide by the random max to get an decimal 
			float r = float(rand())/RAND_MAX;
			color[z] = im(x,y,z) * (1 - noise/2 + noise * r); 
		}
    	brush(out, x, y, color, brush_stroke); 
	}
	return;
}

void singleScalePaintImportance(const Image &im, const Image &importance,
						Image &out, const Image &texture, int size, int N, float noise) {
	// Create painted rendering but vary the density of the strokes according to
	// an importance map
	// // --------- HANDOUT  PS12 ------------------------------
	//scale the brushstroke to have max size 
	float scale = (float) size/ max(texture.height(), texture.width()); 
	Image brush_stroke = scaleLin(texture, scale);

	//Then we will splat a brushstroke on N random locations 
	while(N > 0){
		//choose a sample location
		//make sure that the random location is within the image
		int x = rand() % im.width(); 
    	int y = rand() % im.height(); 
		vector<float> color(im.channels(), 0.0); 
		float r = float(rand())/RAND_MAX;
		if(r < importance(x,y)){
			for(int z = 0; z < im.channels(); z++){
				//divide by the random max to get an decimal 
				color[z] = im(x,y,z) * (1 - noise/2 + noise * r); 
			}
			brush(out, x, y, color, brush_stroke); 
			N--; 
		}
	}

	return;
}

Image sharpnessMap(const Image &im, float sigma) {
	// Calculate sharpness mask 
	// // --------- HANDOUT  PS12 ------------------------------
	Image map(im.width(), im.height(), im.channels()); 

	//get luminance
	vector<Image> lumichrom = lumiChromi(im); 
	//apply gaussian blur 
	Image gaussBlur = gaussianBlur_separable(lumichrom.at(0), sigma); 
	//substract blurred lum by orig lum 
	Image highFreq = lumichrom.at(0) - gaussBlur; 
	//square the high frequency 
	Image squared_highFreq = highFreq * highFreq; 
	//apply gaussian blur wiht sigma*4
	Image gauss_highFreq = gaussianBlur_separable(squared_highFreq,4*sigma); 
	//divide by maximum value
	map = gauss_highFreq / gauss_highFreq.max(); 

	return map;
}

void painterly(const Image &im, Image &out, const Image &texture, int N, int size, float noise) {
	// Create painterly rendering using a first layer of coarse strokes followed
	// by smaller strokes in high detail areas
	// // --------- HANDOUT  PS12 ------------------------------

	//first layer of coarse strokes
	Image first_layer (im.width(), im.height(), im.channels()); 
	first_layer = first_layer + 1; 

	//second layer of high detailed strokes
	Image second_layer = sharpnessMap(im);
	singleScalePaintImportance(im, first_layer, out, texture, size, N, noise); 
	singleScalePaintImportance(im, second_layer, out, texture, size, N, noise);  
	return;
}

Image computeTensor(const Image &im, float sigmaG, float factorSigma) {
 	// Compute xx/xy/yy Tensor of an image. (stored in that order)
 	// // --------- HANDOUT  PS07 ------------------------------
 	vector<Image> LumChr = lumiChromi(im); 
  	Image lum_blur = gaussianBlur_separable(LumChr.at(0), sigmaG); 
  	Image gradient_x = gradientX(lum_blur);
  	Image gradient_y = gradientY(lum_blur);

  	Image tensor = Image(im.width(), im.height(), im.channels()); 
  	for(int x = 0; x < im.width(); x++){
    	for(int y = 0; y < im.height(); y++){
		tensor(x,y,0) += pow(gradient_x(x,y), 2);
		tensor(x,y,1) += gradient_x(x,y) * gradient_y(x,y);
		tensor(x,y,2) += pow(gradient_y(x,y), 2); 
    	}
  	}
 	Image output = gaussianBlur_separable(tensor, sigmaG * factorSigma); 

  	return output;
}


Image testAngle(const Image &im, float sigmaG, float factor) {
	// Extracts orientation of features in im. Angles should be mapped
	// to [0,1]
	// // --------- HANDOUT  PS12 ------------------------------

	//structure tensor at a slightly large scale 
	Image output(im.width(), im.height(), 1); 
	Image structureTensor = computeTensor(im, sigmaG, factor); 
	
	for(int x = 0; x < im.width(); x++){
		for(int y = 0; y < im.height(); y++){
			Matrix m (2,2); 
			m << structureTensor(x,y,0), structureTensor(x,y,1), 
					structureTensor(x,y,1), structureTensor(x,y,2); 
			
			auto solver = Eigen::EigenSolver<Matrix> ();  

			solver.compute(m);

			auto vectors = solver.eigenvectors();
			auto values = solver.eigenvalues();  

			auto eigenvalues1 = values(0).real();
			auto eigenvalues2 = values(1).real();

			float angle = 0;

			if(abs(eigenvalues1) < abs(eigenvalues2)){
				angle = atan2(vectors(1).real(), vectors(0).real());
			}else{
				angle = atan2(vectors(3).real(), vectors(2).real());
			}

			if(angle <= 0){
				angle += 2*M_PI; 
			}
			
			angle = angle / (2 * M_PI);

			output(x,y) = 1 - angle; 
		}
	}

    return output;

}

vector<Image> rotateBrushes(const Image &im, int nAngles) {
	// helper function
	// Returns list of nAngles images rotated by 1*2pi/nAngles
	// // --------- HANDOUT  PS12 ------------------------------
	vector<Image> output; 

	for(int i = 0; i < nAngles; i++){
		float theta = (2*i*M_PI)/nAngles; 
		output.push_back(rotate(im, theta)); 
	}
	return output;
}


void singleScaleOrientedPaint(const Image &im, const Image &importance,
		Image &out, const Image &tensor, const Image &texture,int size, int N, 
		float noise, int nAngles) {
	// Similar to singleScalePaintImportant but brush strokes are oriented
	// according to tensor
	// // --------- HANDOUT  PS12 ------------------------------

	srand(0); //initialize the random function 

	//scale the brushstroke to have max size 
	float scale = (float) size/ max(texture.height(), texture.width()); 
	Image brush_stroke = scaleLin(texture, scale);

	Image angles = testAngle(im);

	vector<Image> rotatedBrushes = rotateBrushes(brush_stroke, nAngles); 

	while(N > 0){
		int x = rand() % im.width();
		int y = rand() % im.height(); 

		int r = rand() / RAND_MAX; 

		if(r < importance(x,y)){
			vector<float> color(im.channels(), 0.0); 
			for(int z = 0; z < im.channels(); z++){
				//divide by the random max to get an decimal 
				float r = float(rand())/RAND_MAX;
				color[z] = im(x,y,z) * (1 - noise/2 + noise * r); 
			}
			int index = int(nAngles * angles(x,y)) % nAngles;
			brush(out, x, y, color, rotatedBrushes[index]);
			N--; 
		}
	}
	return;
}

void orientedPaint(const Image &im, Image &out, const Image &texture, int N, int size, float noise) {
	// Similar to painterly() but strokes are oriented along the directions of maximal structure
	// // --------- HANDOUT  PS12 ------------------------------
	//first layer of coarse strokes

	Image tensor = computeTensor(im);

	Image first_layer (im.width(), im.height(), im.channels()); 
	first_layer = first_layer + 1; 

	//second layer of high detailed strokes
	Image second_layer = sharpnessMap(im);

	singleScaleOrientedPaint(im, first_layer, out, tensor, texture, size, N, noise);
	singleScaleOrientedPaint(im, second_layer, out, tensor, texture, size, N, noise); 

	return;

}




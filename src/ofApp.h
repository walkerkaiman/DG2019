#pragma once

#include "ofMain.h"
#include "ofxLiquidFun.h"
#include "ofxSpout2Sender.h"
#include "ofxCereal.h"
#include "ofxSimpleSerial.h"
//#include "ofxOpenCv.h"

using namespace ofxCereal;

struct Parameters {
	// These are default values. These will be overwritten by loading from JSON file.
	string  serialPort              = "COM4";
	int     baudRate			    = 9600;
	int     serialRequestFrequency  = 50;
	int     particlePopulation	    = 5000;
	float   sensorMin               = 0.0;
	float   sensorMax			    = 180.0;
	float   spriteSize			    = 32.0;
	float   particleLifetimeSeconds = 0.0;
	float   colliderSize			= 10.0;
	float   gravityIntensity		= 2.0;
	float   reticleSize				= 10;
	float   reticleStrokeWeight	    = 3;
	string  spriteFilename			= "particleSprite.png";
	string  linesFilename			= "lines.txt";
	string  spoutServerName			= "Fluid Simulator";

	OFX_CEREAL_DEFINE(	
						CEREAL_NVP(serialPort), 
						CEREAL_NVP(baudRate),
						CEREAL_NVP(serialRequestFrequency),
						CEREAL_NVP(particlePopulation),
						CEREAL_NVP(sensorMin),
						CEREAL_NVP(sensorMax),
						CEREAL_NVP(spriteSize), 
						CEREAL_NVP(particleLifetimeSeconds), 
						CEREAL_NVP(colliderSize), 
						CEREAL_NVP(gravityIntensity),
						CEREAL_NVP(reticleSize),
						CEREAL_NVP(reticleStrokeWeight),
						CEREAL_NVP(spriteFilename),
						CEREAL_NVP(linesFilename),
						CEREAL_NVP(spoutServerName)
					)
};

class ofApp : public ofBaseApp{
	Parameters parameters;

	// Applicaiton Constants
	const int     FRAME_RATE          = 60;
	const ofColor RETICLE_COLOR       = ofColor::red;
	const ofColor BACKGROUND_COLOR    = ofColor::black;
	const string  PARAMETERS_FILENAME = "parameters.json";

	// Logic Variables
	bool isEditMode   = false;
	bool drawing      = false;
	bool pDrawing     = false;
	bool drawingGuide = false;

	public:
		// Simple Serial
		ofxSimpleSerial serial;
		void setGravityFromSensor(string & message);

		// Liquid Fun
		ofxBox2d box2d;
		ofxBox2dParticleSystem particles;
		vector <ofPolyline> lines;
		vector <shared_ptr<ofxBox2dEdge>> edges;

		// Spout
		ofxSpout2::Sender spoutServer;
		ofFbo videoBuffer;

		// Program specific functions
		void loadLinesFromFile();
		void saveLinesToFile();
		void createEdgeFromLine();
		void displayReticle(const float & size, const float & width, const ofColor & color);
		void displayColliders();
		void loadJSON(const string & filename);
		void saveJSON(const string & filename);

		// Standard functions
		void setup();
		void update();
		void draw();
		void exit();
		void keyReleased(int key);
		void mousePressed(int x, int y, int button);
};

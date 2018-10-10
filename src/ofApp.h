#pragma once

#include "ofMain.h"
#include "ofxLiquidFun.h"
#include "ofxSpout2Sender.h"

class ofApp : public ofBaseApp{

	public:
        // Standard functions
		void setup();
		void update();
		void draw();
		void exit();
		void keyReleased(int key);
        void mousePressed(int x, int y, int button);

		// Logic Variables
		bool isEditMode = false;
		bool drawing = false;
		bool pDrawing = false;
		bool drawingGuide = false;
		int  pGravitySelector = 0;

		// Box2D
		ofxBox2d box2d;
		ofxBox2dParticleSystem particles;

		vector <ofPolyline> lines;
		vector <shared_ptr<ofxBox2dEdge>> edges;

		// Spout
		ofxSpout2::Sender spout;
		ofFbo videoBuffer;

		/////
		// PARAMETERS
		/////

		// Particles
		const int   MAX_PARTICLE_POPULATION = 8000;
		const int   INITIAL_PARTICLE_POPULATION = MAX_PARTICLE_POPULATION;
		const int   GRAVITY_FLIP_RATE_SECONDS = 10;
		const float SPRITE_SIZE = 32.0;
		const float PARTICLE_LIFETIME_SECONDS = 0.0;
		const float COLLIDER_SIZE = 6.0;
		const float PARTICLE_SIZE = 32.0;
		const float GRAVITY_INTENSITY = 1.0;

		// Applicaiton
		const int   FRAME_RATE = 60;
		
        // Program specific functions
        bool isFlippingGravity();
        void flipGravity();
        void loadLinesFromFile();
        void saveLinesToFile();
        void createEdgeFromLine();
        void displayReticle(float size, float width, ofColor color);
};

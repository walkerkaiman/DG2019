#include "ofApp.h"
#include <fstream>

//--------------------------------------------------------------
void ofApp::setup(){
    // Window Setup
    ofSetVerticalSync(true);
    ofSetFrameRate(FRAME_RATE);
    ofBackground(ofColor::white);
    
    // Box2D Setup
    box2d.init();
    box2d.setFPS(FRAME_RATE/2);
    box2d.createBounds();
    flipGravity();
    
    // Liquid Fun Setup
    particles.loadImage("particleSprite.png");
    particles.setup(box2d.getWorld(), MAX_PARTICLE_POPULATION, PARTICLE_LIFETIME_SECONDS, COLLIDER_SIZE, PARTICLE_SIZE, ofColor::white);
    particles.setParticleFlag(b2_viscousParticle);

    loadLinesFromFile();
    
    ofVec2f velocity(0, 0);
    
    for (int i = 0; i < INITIAL_PARTICLE_POPULATION; i++) {
        ofVec2f position(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
        
        if (ofRandom(1) > .3) {
            particles.setColor(ofColor::pink);
        }else {
            particles.setColor(ofColor::teal);
        }
        
        particles.createParticle(position.x, position.y, velocity.x, velocity.y);
    }

	// Spout Setup
	screenBuffer.allocate(ofGetScreenWidth(), ofGetScreenHeight());
	screenBuffer.begin();
	ofClear(ofColor::white);
	screenBuffer.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    box2d.update();
    
    if (isFlippingGravity()) {
        flipGravity();
    }

	screenBuffer.begin();
	ofClear(ofColor::black);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	particles.draw();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	if (isEditMode) {
		displayReticle(10, 3, ofColor(255, 0, 0, 127));

		ofSetColor(ofColor::red);

		// Draw a line from the last vertex of line to the current mouse position.
		if (lines.size() > 0) {
			int lastLineSize = lines.back().size();

			if (lastLineSize > 0 && drawingGuide) {
				ofPoint mouse;
				mouse.set(mouseX, mouseY);

				ofPoint lastPoint = lines.back().getVertices()[lastLineSize - 1];

				ofPolyline tempLine;
				tempLine.addVertex(lastPoint);
				tempLine.addVertex(mouse);

				tempLine.draw();
			}
		}

		// Draws a line for all collider edges
		for (auto line : lines) {
			line.draw();
		}
	}
	screenBuffer.end();
	spout.sendTexture(screenBuffer.getTexture(), "Fluid Simulator");
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(255);
	ofClear(ofColor::black);
	screenBuffer.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == ' ') {
        isEditMode =! isEditMode;
        drawing = isEditMode;
        
        if (isEditMode) {
            drawingGuide = false;
        }
    }
    else if (key == 'g' || key == 'G') {
        flipGravity();
    }
    else if (key == OF_KEY_DEL || key == OF_KEY_BACKSPACE) {
        if (isEditMode && lines.size() > 0 && edges.size() > 0) {
            lines.pop_back();
            edges.pop_back();
            saveLinesToFile();
        }
    }
}

void ofApp::mousePressed(int x, int y, int button) {
    if (isEditMode) {
        if (drawing != pDrawing) {
            lines.push_back(ofPolyline());
            drawing = pDrawing;
        }
        
        lines.back().addVertex(x, y);
        createEdgeFromLine ();
        drawingGuide = true;
    }
}

void ofApp::createEdgeFromLine () {
    shared_ptr <ofxBox2dEdge> newEdge = shared_ptr <ofxBox2dEdge> (new ofxBox2dEdge);
    lines.back().simplify();
    
    for (int i = 0; i < lines.back().size(); i++) {
        newEdge.get() -> addVertex(lines.back()[i]);
    }
    
    newEdge.get() -> create(box2d.getWorld());
    edges.push_back(newEdge);
    
    saveLinesToFile();
}

void ofApp::loadLinesFromFile() {
    fstream fin;
    string fileName = ofToDataPath("lines.txt");
    fin.open(fileName.c_str(), ios::in);
    
    if(fin.is_open()) {
        lines.clear();
        vector <string> strLines;
        
        while (!fin.eof()) {
            string ptStr;
            getline(fin, ptStr);
            strLines.push_back(ptStr);
        }
        
        for (int i = 0; i < strLines.size(); i++) {
            vector <string> pts = ofSplitString(strLines[i], ",");
            
            if (pts.size() > 0) {
                lines.push_back(ofPolyline());
                
                for (int j = 0; j < pts.size(); j += 2) {
                    
                    if (pts[j].size() > 0) {
                        float x = ofToFloat(pts[j]);
                        float y = ofToFloat(pts[j+1]);
                        
                        lines.back().addVertex(x, y);
                    }
                    createEdgeFromLine();
                }
            }
        }
    }
    
    fin.close();
}

void ofApp::saveLinesToFile() {
    ofstream f;
    f.clear();
    f.open(ofToDataPath("lines.txt").c_str());
    
    for (int i = 0; i < lines.size(); i++) {
        for (int j = 0; j < lines[i].size(); j++) {
            float x = lines[i][j].x;
            float y = lines[i][j].y;
            f << x << "," << y << ",";
        }
        
        f << "\n";
    }
    f.close();
}

bool ofApp::isFlippingGravity() {
    return ofGetFrameNum() % (GRAVITY_FLIP_RATE_SECONDS * FRAME_RATE) == 0;
}

void ofApp::displayReticle(float size, float width, ofColor color) {
    ofVec2f offset (mouseX, mouseY);
    ofSetColor(color.r, color.g, color.b, color.a);
    ofSetLineWidth(width);
    
    ofPolyline vertical;
    vertical.addVertex(offset.x, offset.y-size);
    vertical.addVertex(offset.x, offset.y+size);
    vertical.draw();
    
    ofPolyline horizontal;
    horizontal.addVertex(offset.x-size, offset.y);
    horizontal.addVertex(offset.x+size, offset.y);
    horizontal.draw();
}

void ofApp::flipGravity() {
    // Randomly select one of the four walls
    int selector = (int)ofRandom(4);
    
    // This while loop will make sure that the gravity is different each time it flips.
    while (selector == pGravitySelector) {
        selector = (int)ofRandom(4);
    }
    
    ofVec2f gravityDirection = ofVec2f(0, 0);
    
    switch(selector) {
        case 0: // Gravity Down
            gravityDirection.y = 1;
            break;
        case 1: // Gravity Up
            gravityDirection.y = -1;
            break;
        case 2: // Gravity Right
            gravityDirection.x = 1;
            break;
        case 3: // Gravity Left
            gravityDirection.x = -1;
            break;
    }
    
    ofVec2f gravityVector = ofVec2f(gravityDirection.x, gravityDirection.y) * GRAVITY_INTENSITY;
    box2d.setGravity(gravityVector);
    pGravitySelector = selector;
}

void ofApp::exit() {
	spout.exit();
}

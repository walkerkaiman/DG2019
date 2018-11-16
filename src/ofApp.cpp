#include "ofApp.h"
#include <fstream>

//--------------------------------------------------------------
void ofApp::setup() {
	loadJSON(PARAMETERS_FILENAME);

    // Window Setup
    ofSetVerticalSync(true);
    ofSetFrameRate(FRAME_RATE);

	// Simple Serial Setup
	serial.setup(parameters.serialPort, parameters.baudRate);
	serial.startContinuousRead();
	ofAddListener(serial.NEW_MESSAGE, this, &ofApp::setGravityFromSensor);

    // Box2D Setup
    box2d.init();
    box2d.setFPS(FRAME_RATE/2);
	box2d.setGravity(parameters.gravityIntensity, 0);
    box2d.createBounds();
    
    // Liquid Fun Setup
	loadLinesFromFile();

    particles.loadImage(parameters.spriteFilename);
    particles.setParticleFlag(b2_elasticParticle);
	particles.setup(box2d.getWorld(), parameters.particlePopulation, parameters.particleLifetimeSeconds, parameters.colliderSize, parameters.spriteSize, ofColor::white);

    const ofVec2f VELOCITY(0, 0);
    
    for (int i = 0; i < parameters.particlePopulation; i++) {
        ofVec2f position(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
        particles.createParticle(position.x, position.y, VELOCITY.x, VELOCITY.y);
    }

	// Spout Setup

}

//--------------------------------------------------------------
void ofApp::update(){
	if (ofGetFrameNum() % parameters.serialRequestFrequency == 0) {
		serial.sendRequest();
	}

    box2d.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofClear(BACKGROUND_COLOR);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    particles.draw();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    if (isEditMode) {
        displayColliders();
    }
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
    string fileName = ofToDataPath(parameters.linesFilename);
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
	bool successfullyRemovedOldFile = ofFile::removeFile(parameters.linesFilename, true);

	if (successfullyRemovedOldFile) {
		ofstream f;
		f.clear();
		f.open(ofToDataPath(parameters.linesFilename).c_str());

		for (int i = 0; i < lines.size(); i++) {
			for (int j = 0; j < lines[i].size(); j++) {
				float x = lines[i][j].x;
				float y = lines[i][j].y;
				f << x << "," << y << ",";
			}

			f << "\n";
		}

		f.close();
		ofLog(OF_LOG_NOTICE, "Created new save with collider data.");
	}else{
		ofLog(OF_LOG_WARNING, "Was not able to remove old collider data file.");
	}
}

void ofApp::displayReticle(const float & size, const float & width, const ofColor & color) {
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

void ofApp::displayColliders() {
	displayReticle(parameters.reticleSize, parameters.reticleStrokeWeight, RETICLE_COLOR);

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

void ofApp::exit() {
	saveJSON(PARAMETERS_FILENAME);
	//spoutServer.exit();
}

void ofApp::loadJSON(const string & filename) {
	ofFile file(filename);

	if (file.exists()) {
		jsonin ji(file);
		ji >> parameters;
	}
	else {
		ofLog(OF_LOG_WARNING, "JSON file not found. Using default values...");
	}
}

void ofApp::saveJSON(const string & filename) {
	ofFile file(filename, ofFile::WriteOnly);
	jsonout jo(file);
	jo << parameters;
}

void ofApp::setGravityFromSensor(string & message) {
	float sensorAngle = ofToFloat(message);
	float gravityDirection = ofMap(sensorAngle, parameters.sensorMin, parameters.sensorMax, -parameters.gravityIntensity, parameters.gravityIntensity, true);
	box2d.setGravity(ofVec2f(-gravityDirection, 0));

	if (isEditMode) { ofLog(OF_LOG_NOTICE, message); }
}

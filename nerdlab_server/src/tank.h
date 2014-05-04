//
//  tank.h
//  tanks
//
//  Created by Clay Ewing on 4/18/14.
//
//

#include "ofMain.h"
#include "ofxBox2dRect.h"
#include "custom.h"
#include "shot.h"
#define SHOT_SPEED  10
#define STARTING_ARMOR 10

class Tank : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    void shoot();
    int armor;
    ofColor color;
    ofImage *image;
    ofImage *glow;
    ofImage bulletImage;
    string ips[3];
    vector <ofPtr<Shot> > shots;
    
};

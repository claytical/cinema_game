//
//  tank.h
//  tanks
//
//  Created by Clay Ewing on 4/18/14.
//
//

#include "ofMain.h"
#include "ofxBox2dRect.h"
#include "ofxBox2d.h"
#include "custom.h"
#include "shot.h"

class Tank : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    void shoot(ofxBox2d *b2d);
    ofColor color;
    ofImage *image;
    string ips[3];
    vector <ofPtr<Shot> > shots;
    
};

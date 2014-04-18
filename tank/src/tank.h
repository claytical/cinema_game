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

class Tank : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    ofColor color;
    ofImage *image;
    string ips[3];
    
};

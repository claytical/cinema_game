//
//  shot.h
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "ofMain.h"
#include "ofxBox2dCircle.h"
#include "custom.h"

class Shot : public ofxBox2dCircle {
    
public:
    void display();
    void setupCustom(int id);
    bool remove;
    ofColor color;
    ofImage *image;
    
};

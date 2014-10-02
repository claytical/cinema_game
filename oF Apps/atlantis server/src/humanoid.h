//
//  humanoid.h
//  atlantis server
//
//  Created by Clay Ewing on 3/28/14.
//
//

#include "ofMain.h"
#include "ofxBox2dRect.h"

#include "custom.h"

class Humanoid : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom();
    ofColor color;
    ofImage *image;
};

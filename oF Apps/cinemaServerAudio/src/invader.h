//
//  invader.h
//  cinemaServerAudio
//
//  Created by Clay Ewing on 3/4/14.
//
//

#include "ofMain.h"
#include "ofxBox2dRect.h"
#include "custom.h"

class Invader : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    ofImage *image;
    
};

//
//  shot.h
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//
#ifndef __SHOT_H_INCLUDED__
#define __SHOT_H_INCLUDED__

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
    ofPoint velocity;
    
};
#endif
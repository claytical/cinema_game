//
//  food.h
//  cinemaServer
//
//  Created by Clay Ewing on 2/13/14.
//
//

#include "ofMain.h"
#include "ofxBox2dRect.h"
#include "custom.h"

class Food : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    bool eaten;
    ofColor color;
    ofImage *image;

};

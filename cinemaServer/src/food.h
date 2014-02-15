//
//  food.h
//  cinemaServer
//
//  Created by Clay Ewing on 2/13/14.
//
//

#include "ofMain.h"
#include "ofxBox2dCircle.h"
#include "custom.h"

class Food : public ofxBox2dCircle {
    
public:
    void display();
    void setupCustom(int id);
    bool eaten;
};

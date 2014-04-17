//
//  tank.h
//  atlantis
//
//  Created by Clay Ewing on 4/6/14.
//
//
#include "ofMain.h"

class Tank  {
    
public:
    void create(float x, float y, int numControllers);
    void display();
    void move(int controllerIndex);
    ofImage image;
    ofPoint position;
    ofPoint velocity;
    float rotation;
    int numberOfControllers;
    int index;
};

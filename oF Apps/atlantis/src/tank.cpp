//
//  tank.cpp
//  atlantis
//
//  Created by Clay Ewing on 4/6/14.
//
//

#include "tank.h"

void Tank::create(float x, float y, int numControllers) {
    position.set(x, y);
    velocity.set(5,0);
    index = 0;
    numberOfControllers = numControllers;
}

void Tank::move(int controllerIndex) {
    if (controllerIndex%numberOfControllers == index) {
        position += velocity;
        index++;
        if (index >= numberOfControllers) {
            index = 0;
        }
    }
}


void Tank::display() {
    //ofSetColor(255);
    ofRect(position.x, position.y, 50, 50);
}
//
//  food.cpp
//  cinemaServer
//
//  Created by Clay Ewing on 2/13/14.
//
//

#include "food.h"

void Food::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = FOOD_TYPE;
    theData->remove = false;
    theData->id = id;
}
void Food::display() {
    float radius = getRadius();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(0,255,0);
    ofFill();
    ofCircle(0,0,radius);
    ofPopMatrix();
}
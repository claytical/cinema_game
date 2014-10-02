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
//    float radius = getRadius();
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(0,255,0);
    ofFill();
    image->draw(0,0, width, height);
//    ofRect(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CENTER);
    //    ofCircle(0,0,radius);
    ofPopMatrix();
}
//
//  humanoid.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "humanoid.h"

void Humanoid::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_HUMANOID;
    theData->remove = false;
    theData->id = id;

    
    //10 = players per team
    switch (id%10) {
        case 0:
            color = ofColor::springGreen;
            break;
        case 1:
            color = ofColor::orchid;
            break;
        case 2:
            color = ofColor::blueViolet;
            break;
        case 3:
            color = ofColor::turquoise;
            break;
        case 4:
            color = ofColor::whiteSmoke;
            break;
        case 5:
            color = ofColor::tomato;
            break;
        case 6:
            color = ofColor::violet;
            break;
        case 7:
            color = ofColor::mintCream;
            break;
        case 8:
            color = ofColor::darkorange;
            break;
        case 9:
            color = ofColor::darkMagenta;
            break;
        default:
            color = ofColor::darkGrey;
            break;
    }

}



void Humanoid::display() {
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
//    ofRotateZ(getRotation());
    ofSetColor(color);
    ofFill();
    image->draw(0,0, width, height);
   // ofRect(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
}
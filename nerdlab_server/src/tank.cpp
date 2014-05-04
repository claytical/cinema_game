//
//  tank.cpp
//  tanks
//
//  Created by Clay Ewing on 4/18/14.
//
//

#include "tank.h"
static bool removeShot(Shot s) {
    return s.remove;
}



void Tank::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_TANK;
    theData->remove = false;
    theData->id = id;
    armor = STARTING_ARMOR;
    bulletImage.loadImage("bullets/bullet.png");
}
void Tank::shoot() {
    ofPtr<Shot> shot = ofPtr<Shot>(new Shot);
    shot.get()->setPhysics(2, .1, 0);
    float xPos = (cos((getRotation()-90) * PI/180)) * ((getHeight()/2) + 10);
    float yPos = (sin((getRotation()-90) * PI/180)) * ((getWidth()/2 + 10));
    
    shot.get()->setup(this->getWorld(), getPosition().x + xPos, getPosition().y + yPos, 3);
    cout << "X: " << cos((getRotation()-90)*PI/180) << endl;
    cout << "Y: " << sin((getRotation()-90)*PI/180) << endl;
    shot.get()->setVelocity(cos((getRotation()-90)*PI/180) * SHOT_SPEED, sin((getRotation()-90)*PI/180) * SHOT_SPEED);
    shot.get()->setData(new CustomData());
    shot.get()->setupCustom(shots.size());

    shot.get()->color = ofColor(color);
    shot.get()->image = &bulletImage;
 //   cout << "X Velocity: " << shot.get()->getVelocity().x << endl;
 //   cout << "Y Velocity: " << shot.get()->getVelocity().y << endl;

    shots.push_back(shot);

    
}
void Tank::display() {
    //ofRemove(shots, removeShot);
    //#1 moves
    
    //#2 rotates
    if (ips[1].empty()) {
        //no player 2
        setRotation(ofGetElapsedTimef()*30);
    }

    //#3 shoots

    if (ips[2].empty()) {
        //no player 3
        if (int(ofRandom(10))%10 == 1) {
            shoot();
        }
    }

    ofPushMatrix();
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    if (armor < 3) {
        float alpha = ofMap(sin(ofGetFrameNum() * .3), -1, 1, 0, 255);
        ofSetColor(255, 0, 0, alpha);
        glow->draw(-getWidth(), -getHeight(), 50, 50);
        ofSetColor(color);
    }
    else {
        ofSetColor(color, ofMap(armor, 0, STARTING_ARMOR, 50, 255));
    }
    ofFill();
    image->draw(-getWidth()/2, -getHeight()/2, getWidth(), getHeight());

    ofPopMatrix();
    
}
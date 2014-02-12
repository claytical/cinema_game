//
//  player.h
//
//  Created by Clay Ewing on 2/8/14.
//
//
#include "ofMain.h"
#include "ofxBox2dRect.h"

class Player : public ofxBox2dRect {
    
public:
    void display();
    string name;
    int playerId;
    int teamId;
    int subTeamId;
    ofColor color;
    ofImage *image;
};

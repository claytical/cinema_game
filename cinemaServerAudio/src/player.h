//
//  player.h
//
//  Created by Clay Ewing on 2/8/14.
//
//
#include "ofMain.h"
#include "ofxBox2dRect.h"

#include "custom.h"

class Player : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    string name;
    int playerId;
    int teamId;
    int subTeamId;
    int score;
    bool hit;
    ofColor color;
    ofImage *image;
};

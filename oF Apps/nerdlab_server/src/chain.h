//
//  chain.h
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "ofMain.h"
#include "ofxBox2dRect.h"
#include "custom.h"

class Chain : public ofxBox2dRect {
    
public:
    void display();
    void setupCustom(int id);
    ofColor color;
    ofImage *image;
    //players assigned part of chain
    //need vector of int for control look up. chainMap[playerID] = chainID
};

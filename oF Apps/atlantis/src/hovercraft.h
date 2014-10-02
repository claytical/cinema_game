//
//  hovercraft.h
//  atlantis
//
//  Created by Clay Ewing on 4/5/14.
//
//

#include "ofMain.h"

class Hovercraft  {
    
public:
    void create(float x, float y, int numPassengers);
    void display();
    void move(int controllerIndex);
    int passengers;
    ofImage image;
    ofPoint position;
    ofPoint velocity;
    int index;
};

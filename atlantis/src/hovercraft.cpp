//
//  hovercraft.cpp
//  atlantis
//
//  Created by Clay Ewing on 4/5/14.
//
//

#include "hovercraft.h"

void Hovercraft::create(float x, float y, int numPassengers) {
    position.set(x, y);
    velocity.set(5,0);
    index = 0;
    passengers = numPassengers;
}

void Hovercraft::move(int passengerIndex) {
    cout << "I'm trying to move " << passengerIndex << " to " << passengers << endl;
    if (passengerIndex%passengers == index) {
        position += velocity;
        index++;
        if (index >= passengers) {
            index = 0;
        }
    }
}


void Hovercraft::display() {
    ofSetColor(255);
    ofRect(position.x, position.y, 50, 50);
}
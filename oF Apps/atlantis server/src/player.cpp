//
//  player.cpp
//  example-Simple
//
//  Created by Clay Ewing on 2/8/14.
//
//

#include "player.h"

void Player::create(string ip) {
    playerIp = ip;
    score = 0;
}

void Player::resetScore() {
    score = 0;
}
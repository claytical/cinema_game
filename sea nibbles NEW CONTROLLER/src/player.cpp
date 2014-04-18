//
//  player.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/28/14.
//
//

#include "player.h"

void Player::create(string _name,string ip) {
    name = _name;
    playerIp = ip;
    score = 0;
    connected = true;

}

void Player::resetScore() {
    score = 0;
}

void Player::assignTeam(int numberOfPlayers) {
    
}
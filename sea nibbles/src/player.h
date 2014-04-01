//
//  player.h
//  atlantis
//
//  Created by Clay Ewing on 3/28/14.
//
//

#include "ofMain.h"

class Player  {
    
public:
    void create(string _name, string ip);
    void resetScore();
    void assignTeam(int numberOfPlayers);
    string name;
    int playerId;
    string playerIp;
    int teamId;
    int subTeamId;
    int score;
};

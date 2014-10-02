//
//  player.h
//
//  Created by Clay Ewing on 2/8/14.
//
//
#include "ofMain.h"
//#include "humanoid.h"

class Player  {
    
public:
    void create(string ip);
    void resetScore();
//    Humanoid avatar;
    string name;
    int playerId;
    string playerIp;
    int teamId;
    int subTeamId;
    int score;
};

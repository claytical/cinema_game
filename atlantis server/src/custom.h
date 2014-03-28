//
//  custom.h
//  cinemaServer
//
//  Created by Clay Ewing on 2/14/14.
//
//
#ifndef __CUSTOM_DATA_H_INCLUDED__
#define __CUSTOM_DATA_H_INCLUDED__

#define PLAYER_TYPE       1
#define FOOD_TYPE         2
#define TYPE_HUMANOID     1
#define TYPE_FOOD         2
#define TYPE_SHIP         3



class CustomData {
public:
    int type;
    bool remove;
    int id;
};

#endif
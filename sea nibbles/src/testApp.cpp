#include "testApp.h"
void testApp::removeExistingPlayer(string ip) {
    for (int i = 0; i < players.size(); i++) {
        if(players[i].playerIp == ip) {
            cout << "Found Existing Player";
            players.erase(players.begin()+i);
            humanoids.erase(humanoids.begin()+i);
            break;
        }
    }
}

static bool removeFood(ofPtr<ofxBox2dBaseShape> shape) {
    CustomData *custom = (CustomData *)shape->getData();
    return custom->remove;
}
//--------------------------------------------------------------
void testApp::setup(){
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    //networking setup
    //bonjour auto discovery stuff
    bonjour = new ofxBonjourIp();
    bonjour->addEventListeners(this);
    bonjour->startService();
    bonjour->discoverService();
    //osc setup
    oscReceiver.setup(SERVER_PORT);
    
    //narrator setup
    narrator.setup("com.apple.speech.synthesis.voice.Fred");

    //box2d setup
    box2d.init();
	box2d.setGravity(0, 0);
    box2d.enableEvents();
	box2d.setFPS(30.0);
    box2d.createBounds();
    ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
    ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);

    
    debugging = true;
    gameStarted = false;
    gameState = GAME_STATE_WAITING;
    gameTimer = 999999999;
    timerText.loadFont("joystix.ttf", 48);
    winnerID = -1;
}

//--------------------------------------------------------------
void testApp::update(){
    //create a container for whatever message we get
    ofRemove(food, removeFood);
    box2d.update();
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(&msg);
        
        //check for 
        if (msg.getAddress() == "/join") {
            string incomingPlayerIP = msg.getRemoteIp();
            string playerName = msg.getArgAsString(0);
            //look for existing players with the same IP
            if (std::find(playerIPs.begin(), playerIPs.end(), incomingPlayerIP) == playerIPs.end()) {
                //no player with that IP address, create a new one
                    int playerID = newPlayer(playerName, incomingPlayerIP);
                
                //join the game
                joinGame(playerID);
                newHumanoid();
               // sendState(incomingPlayerIP, GAME_STATE_PLAYING);
            }
            else {
                //wants to rejoin
                removeExistingPlayer(incomingPlayerIP);
                //create a new player
                int playerID = newPlayer(playerName, incomingPlayerIP);
                
                //join the game
                joinGame(playerID);
                newHumanoid();
                //sendState(incomingPlayerIP, GAME_STATE_PLAYING);
                
            }
        }

        if (msg.getAddress() == "/move") {
            if (msg.getNumArgs() == 3) {
                int playerNumber = msg.getArgAsInt32(0);
                if (players.size() > playerNumber) {
                    float xSpeed = msg.getArgAsFloat(1) * -.1;
                    float ySpeed = msg.getArgAsFloat(2) * -.1;
                    humanoids[playerNumber].get()->setVelocity(xSpeed, ySpeed);
                    
                }
            }
        }
    }

    //replenish food
    
    if (food.size() < AMOUNT_OF_FOOD) {
        for (int i = food.size(); i < AMOUNT_OF_FOOD; i++) {
            newFood();
        }

    }
    
    if (ofGetElapsedTimef() > gameTimer && gameStarted) {
        gameStarted = false;
        winnerID = whoWon();
        for (int i = 0; i < players.size(); i++) {
            cout << players[i].name << " : " << players[i].score << endl;
            players[i].resetScore();
        }
        if (winnerID >= 0) {
            cout << "The winner is " << players[winnerID].name << endl;
        }
        broadcastState(GAME_STATE_WAITING);

    }
}
//--------------------------------------------------------------
void testApp::draw(){
    for (int i = 0; i < humanoids.size(); i++) {
        humanoids[i].get()->display();
    }
    
    for (int i = 0; i < food.size(); i++) {
        food[i].get()->display();
    }

    // draw the ground
    string timeLeft = ofToString(abs(int(ofGetElapsedTimef() - gameTimer)));
    if (gameStarted) {
        if (ofGetElapsedTimef() < gameTimer - 28) {
            ofSetColor(0);
            timerText.drawStringCentered("feed!", ofGetWidth()/2-2, ofGetHeight()/2-2);
            ofSetColor(255);
            timerText.drawStringCentered("feed!", ofGetWidth()/2, ofGetHeight()/2);
            
        }
        ofSetColor(0);
        timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 18, timerText.getLineHeight() + 18);
        ofSetColor(255);
        timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 20, timerText.getLineHeight() + 20);

    }
    else {
        if (winnerID >= 0) {
            ofSetColor(0);
            timerText.drawStringCentered(players[winnerID].name + " wins!", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
            ofSetColor(255);
            timerText.drawStringCentered(players[winnerID].name + " wins!", ofGetWidth()/2, ofGetHeight()/2);
            
        }
        else {
            ofSetColor(0);
            timerText.drawStringCentered("Press Space to Start", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
            ofSetColor(255);
            timerText.drawStringCentered("Press Space to Start", ofGetWidth()/2, ofGetHeight()/2);
            
        }
    }

}

//--------------------------------------------------------------
void testApp::sendState(string ip, int state) {
    ofxOscMessage m;
    m.setAddress("/feedback");
    m.addStringArg("state");
    m.addIntArg(state);
    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::sendControl(string ip, int control, int playerId) {
    //bug in iOS software, need to revise. currently using JOINED address to change the controls
    ofxOscMessage m;
    m.setAddress("/joined");
    m.addIntArg(playerId);
//we're making everyone on the same team and subteam for now
    m.addIntArg(1); //TEAM
    m.addIntArg(1); // SUBTEAM

    
    //this sets the control scheme
    m.addIntArg(control);
    //set them to playing, should be redundant
    m.addIntArg(GAME_STATE_PLAYING);

    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}


//--------------------------------------------------------------
void testApp::broadcastState(int state) {
    for (int i = 0; i < playerIPs.size(); i++) {
        sendState(playerIPs[i], state);
    }
}

//--------------------------------------------------------------
void testApp::broadcastControl(int control) {
    for (int i = 0; i < playerIPs.size(); i++) {
        sendState(playerIPs[i], GAME_STATE_PLAYING);
        sendControl(playerIPs[i], control, players[i].playerId);
        cout << "Sending " << control << " to " << playerIPs[i] << endl;
    }
    
}


//--------------------------------------------------------------
int testApp::newPlayer(string player, string ipaddress) {
    playerIPs.push_back(ipaddress);
    Player p;
    p.create(player, ipaddress);
    players.push_back(p);
    return (players.size() - 1);
}

//--------------------------------------------------------------
void testApp::joinGame(int playerId) {
    ofxOscMessage msg;
    msg.setAddress("/joined");
    msg.addIntArg(playerId);
    //keep team, subteam and control at ... +1 because it hasn't been pushed back yet
//    msg.addIntArg(playerId%10);
//    msg.addIntArg(playerId%players.size());
    msg.addIntArg(1); //TEAM
    msg.addIntArg(1); // SUBTEAM

    cout << "Team " << playerId%10 << " Sub " << playerId%players.size() << endl;
    //default state is dragging
    msg.addIntArg(GAME_CONTROL_MOVE);
    msg.addIntArg(GAME_STATE_PLAYING);
    oscSender.setup(players[playerId].playerIp, CLIENT_PORT);
    msg.setRemoteEndpoint(players[playerId].playerIp, CLIENT_PORT);
    oscSender.sendMessage(msg);
    cout << "Sending Join Message to " << players[playerId].playerIp << endl;
}



//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if (key == ' ') {
        gameStarted = true;
        broadcastState(GAME_STATE_PLAYING);
        gameTimer = ofGetElapsedTimef() + 30;
    }
 }

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void testApp::onPublishedService(const void* sender, string &serviceIp) {
    ofLog() << "Received published service event: " << serviceIp;
}

void testApp::onDiscoveredService(const void* sender, string &serviceIp) {
    ofLog() << "Received discovered service event: " << serviceIp;
}

void testApp::onRemovedService(const void* sender, string &serviceIp) {
    ofLog() << "Received removed service event: " << serviceIp;
}

//-----------GAME #1 FUNCTIONS

void testApp::newFood() {
    ofPtr<Food> f = ofPtr<Food>(new Food);
    f.get()->setPhysics(.5, 1, 1);
    
    float fSize = ofRandom(.1, .5);
    int planktonNumber = int(ofRandom(5));
    // f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 50 * fSize, 50 * fSize);
    
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));
    //f.get()->image = &plankton[planktonNumber];
    
    f.get()->setData(new CustomData());
    f.get()->setupCustom(food.size());
    food.push_back(f);
    
}

void testApp::newHumanoid() {
    ofPtr<Humanoid> h = ofPtr<Humanoid>(new Humanoid);
    h.get()->setPhysics(.5, 1, 1);
    
    // f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    h.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 50, 50);
    
    h.get()->setVelocity(0,0);
    //f.get()->image = &plankton[planktonNumber];
    
    h.get()->setData(new CustomData());
    h.get()->setupCustom(humanoids.size());
    humanoids.push_back(h);
    cout << "Humanoid Created" << endl;
    
}

//--------------------------------------------------------------
void testApp::contactStart(ofxBox2dContactArgs &e) {
    cout  << "made contact" << endl;
    
    if(e.a != NULL && e.b != NULL) {
        CustomData *data1 = (CustomData *)e.a->GetBody()->GetUserData();
        CustomData *data2 = (CustomData *)e.b->GetBody()->GetUserData();
        if (data1 != NULL && data2 != NULL) {
            if (data1->type == data2->type) {
                //colliding against each other
                if (data1->type == TYPE_HUMANOID) {
                    //PLAYERS COLLIDING WITH EACH OTHER
                }
                if (data1->type == TYPE_FOOD) {
                    //FOOD ON FOOD COLLISION
                }
            }
            else {
                //two different types colliding, lets figure it out
                //if more than 2 types, needs more logic
                if (data1->type == TYPE_FOOD) {
                    //food is data1, player is data2
                    data1->remove = true;
                    players[data2->id].score+= food[data1->id].get()->getWidth();
                    // newFood();
                }
                else {
                    //food is data2, player is data1
                    data2->remove = true;
                    players[data1->id].score+= food[data2->id].get()->getWidth();
                }
                
            }
        }
    }
}

//--------------------------------------------------------------
void testApp::contactEnd(ofxBox2dContactArgs &e) {
    if (e.a != NULL && e.b != NULL) {
    }
    
}

int testApp::whoWon() {
    int highScore = -999;
    int theWinner = -1;
    for (int i = 0; i < players.size(); i++) {
        if (players[i].score > highScore) {
            theWinner = i;
            highScore = players[i].score;
        }
    }
    return theWinner;
}


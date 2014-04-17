#include "testApp.h"
int testApp::removeExistingPlayer(string ip) {
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
    ofBackground(10);
    collectFx.loadSound("collect.wav");
    backgroundTrack.loadSound("background.wav");
    backgroundTrack.setLoop(true);
    backgroundTrack.play();
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

    //game setup
    
    for (int i = 0; i < 9; i++) {
        playerImages[i].loadImage(ofToString(i) + ".png");
    }
    plankton.loadImage("plankton.png");
    debugging = true;
    gameStarted = false;
    gameState = GAME_STATE_WAITING;
    gameTimer = 999999999;
    timeUntilNextGame = 5000;
    timerText.loadFont("joystix.ttf", 48);
    winnerID = -1;
    imageCounter = 0;
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
//                imageCounter++;

                // sendState(incomingPlayerIP, GAME_STATE_PLAYING);
            }
            else {
                //wants to rejoin

                int removedPlayerID = removeExistingPlayer(incomingPlayerIP);
                //create a new player
                int playerID = newPlayer(playerName, incomingPlayerIP);
                
                //join the game
//                imageCounter--;

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
        }
        if (winnerID >= 0) {
            cout << "The winner is " << players[winnerID].name << endl;
        }
        broadcastState(GAME_STATE_WAITING);
        timeUntilNextGame = ofGetElapsedTimef() + 30;
        ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactStart);
        ofRemoveListener(box2d.contactEndEvents, this, &testApp::contactEnd);

    }
    //restart game automatically
    if (!gameStarted && ofGetElapsedTimef() > timeUntilNextGame) {

        gameStarted = true;
        broadcastState(GAME_STATE_PLAYING);
        gameTimer = ofGetElapsedTimef() + 30;
        for (int i = 0; i < players.size(); i++) {
            players[i].resetScore();
        }
        ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
        ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);

    }
}
//--------------------------------------------------------------
void testApp::draw(){
    for (int i = 0; i < humanoids.size(); i++) {
//        ofSetColor(127);
//        ofDrawBitmapString(ofToString(players[i].score), humanoids[i].get()->getPosition().x, -humanoids[i].get()->getPosition().y);

        humanoids[i].get()->display();
    
    }
    
    for (int i = 0; i < food.size(); i++) {
        food[i].get()->display();
    }

    // draw the ground
    string timeLeft = ofToString(abs(int(ofGetElapsedTimef() - gameTimer)));
    string timeTil = ofToString(abs(int(ofGetElapsedTimef() - timeUntilNextGame)));
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
            timerText.drawStringCentered(players[winnerID].name + " wins!", ofGetWidth()/2 - 2, ofGetHeight()/3 - 2);
            timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.5 -2);

            ofSetColor(255);
            timerText.drawStringCentered(players[winnerID].name + " wins!", ofGetWidth()/2, ofGetHeight()/3);
            timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2, ofGetHeight()/2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.5);

        }
        else {
            ofSetColor(0);
            timerText.drawStringCentered("Press Space to Start", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.5 -2);
            ofSetColor(255);
            timerText.drawStringCentered("Press Space to Start", ofGetWidth()/2, ofGetHeight()/2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.5);
            
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
//for larger game, update needs to be made to controller to assign color, not by int to allow for scalability and flexibility
    msg.addIntArg(imageCounter%10); //TEAM - color
    //shape #1
    if (imageCounter < 10) {
        msg.addIntArg(0); //SUBTEAM - shape
    }
    else if (imageCounter < 20) {
        msg.addIntArg(1); //SUBTEAM - shape
        
    }
    else if (imageCounter < 30) {
        msg.addIntArg(2); //SUBTEAM - shape
        
    }
    else if (imageCounter < 40) {
        msg.addIntArg(3); //SUBTEAM - shape
        
    }
    
    else if (imageCounter < 50) {
        msg.addIntArg(4); //SUBTEAM - shape
        
    }
    else if (imageCounter < 60) {
        msg.addIntArg(5); //SUBTEAM - shape
        
    }
    else if (imageCounter < 70) {
        msg.addIntArg(6); //SUBTEAM - shape
        
    }
    else if (imageCounter < 80) {
        
        msg.addIntArg(7); //SUBTEAM - shape
    }
    
    else if (imageCounter < 90) {
        msg.addIntArg(8); //SUBTEAM - shape
        
    }
    else {
        msg.addIntArg(9); //SUBTEAM - shape
        
    }
    

    cout << "Team " << imageCounter%10 << " Sub " << imageCounter%players.size() << endl;
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
        for (int i = 0; i < players.size(); i++) {
            players[i].resetScore();
        }
        ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
        ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);

    }
    if (key == 'r' && !gameStarted) {
        winnerID = -1;
        players.clear();
        humanoids.clear();
        playerIPs.clear();
        imageCounter = 0;
    }
    if (key == 'n') {
        newPlayer("zombie" + ofToString(imageCounter), ofToString(ofRandom(5)));
        newHumanoid();
        cout << "Humanoids:" << humanoids.size() << endl;
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
    //int planktonNumber = int(ofRandom(5));
    // f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 50 * fSize, 50 * fSize);
    
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));
    f.get()->image = &plankton;
    
    f.get()->setData(new CustomData());
    f.get()->setupCustom(food.size());
    food.push_back(f);
    
}

void testApp::newHumanoid() {
    ofPtr<Humanoid> h = ofPtr<Humanoid>(new Humanoid);
    h.get()->setPhysics(.5, 1, 1);
    
    // f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    h.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), playerImages[0].width/2, playerImages[0].height/2);
    
    h.get()->setVelocity(0,0);
    //f.get()->image = &plankton[planktonNumber];
    
    h.get()->setData(new CustomData());
    h.get()->setupCustom(humanoids.size());
    if (imageCounter < 10) {
        h.get()->image = &playerImages[0];
    }
    else if (imageCounter < 20) {
        h.get()->image = &playerImages[1];
    }
    else if (imageCounter < 30) {
        h.get()->image = &playerImages[2];
    }
    else if (imageCounter < 40) {
        h.get()->image = &playerImages[3];
    }
    else if (imageCounter < 50) {
        h.get()->image = &playerImages[4];
    }
    else if (imageCounter < 60) {
        h.get()->image = &playerImages[5];
    }
    else if (imageCounter < 70) {
        h.get()->image = &playerImages[6];
    }
    else if (imageCounter < 80) {
        h.get()->image = &playerImages[7];
    }
    else if (imageCounter < 90) {
        h.get()->image = &playerImages[8];
    }
    else {
        h.get()->image = &playerImages[9];
        
    }
    imageCounter++;
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
                collectFx.play();
                //two different types colliding, lets figure it out
                //if more than 2 types, needs more logic
                if (data1->type == TYPE_FOOD) {
                    //food is data1, player is data2
                    players[data2->id].score+= food[data1->id].get()->getWidth();

                    data1->remove = true;
                    // newFood();
                }
                else {
                    //food is data2, player is data1
                    players[data1->id].score+= food[data2->id].get()->getWidth();

                    data2->remove = true;
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


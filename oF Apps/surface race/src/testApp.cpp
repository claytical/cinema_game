#include "testApp.h"

int testApp::removeExistingPlayer(string ip) {
    for (int i = 0; i < players.size(); i++) {
        if(players[i].playerIp == ip) {
            cout << "Found Existing Player";
            players.erase(players.begin()+i);
            break;
        }
    }
}

void testApp::setPlayerAlive(string ip) {
    for (int i = 0; i < players.size(); i++) {
        if(players[i].playerIp == ip) {
            cout << "Found Existing Player";
            players[i].connected = true;
            break;
        }
    }
    
}
static bool removeDeadPlayer(Player p) {
    if (!p.connected) {
        cout << "found unconnected player" << endl;
        return true;
    }
    return false;
}
static bool removeShape(ofPtr<ofxBox2dBaseShape> shape) {
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
//    box2d.createBounds();
	box2d.setFPS(30.0);

    //game setup
    podImage.loadImage("vehicles/car.png");
    ofDirectory dir("abstract");
    dir.allowExt("png");
    dir.sort();
    dir.listDir();
    playerImages.clear();
    cout << "cleared current images" << endl;
    imageCounter = 0;

    for (int i = 0; i < dir.numFiles(); i++) {
        ofImage img;
        img.loadImage(dir.getFile(i));
        playerImages.push_back(img);
        cout << "creating new image with " << i << endl;
    }
    debugging = true;
    gameStarted = false;
    gameState = GAME_STATE_WAITING;
    gameTimer = 999999999;
    timeUntilNextGame = 5000;
    timerText.loadFont("joystix.ttf", 48);
    winnerID = -1;
}

//--------------------------------------------------------------
void testApp::update(){
/*
    for (int i = 0; i < tanks.size(); i++) {
        ofRemove(tanks[i]->shots, removeShape);
    }
    ofRemove(tanks, removeTank);
*/
    //create a container for whatever message we get
    box2d.update();
    if (gameStarted) {
        if (attackers.size() < AMOUNT_OF_ATTACKERS) {
            for (int i = attackers.size(); i < AMOUNT_OF_ATTACKERS; i++) {
                newAttacker();
            }
        }
        
    }
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(&msg);
        
        //check for
        if (msg.getAddress() == "/alive") {
            setPlayerAlive(msg.getRemoteIp());
        }
        if (msg.getAddress() == "/join") {
            string incomingPlayerIP = msg.getRemoteIp();
            string playerName = msg.getArgAsString(0);
            //look for existing players with the same IP
            if (std::find(playerIPs.begin(), playerIPs.end(), incomingPlayerIP) == playerIPs.end()) {

                //no player with that IP address, create a new one
                int playerID = newPlayer(playerName, incomingPlayerIP);
                //join the game
                joinGame(incomingPlayerIP);
                //newHumanoid(incomingPlayerIP);
            }
            else {
                //wants to rejoin

                int removedPlayerID = removeExistingPlayer(incomingPlayerIP);
                //create a new player
                int playerID = newPlayer(playerName, incomingPlayerIP);
                joinGame(incomingPlayerIP);
            }
        }
        
        if (msg.getAddress() == "/quit") {
            removeExistingPlayer(msg.getRemoteIp());
        }
        
        if (msg.getAddress() == "/accel") {
            //x and y values
            int chainNum = msg.getArgAsInt32(0);
            int podNum = find(chains[chainNum].playerIps.begin(), chains[chainNum].playerIps.end(),msg.getRemoteIp()) - chains[chainNum].playerIps.begin();
            float xSpeed = msg.getArgAsFloat(1) * -.1;
            float ySpeed = msg.getArgAsFloat(2) * -.1;
            chains[chainNum].pods[podNum]->setVelocity(xSpeed, ySpeed);
            
            
            //           tanks[tankNumber].get()->setVelocity(xSpeed, ySpeed);
            
        }
        
        
    }

    //restart game automatically
    if (!gameStarted && ofGetElapsedTimef() > timeUntilNextGame) {
        startGame();

    }
}
//--------------------------------------------------------------
void testApp::draw(){
    /*
    for (int i = 0; i < tanks.size(); i++) {
        tanks[i].get()->display();
        for (int j = 0; j < tanks[i]->shots.size(); j++) {
            tanks[i]->shots[j]->display();
        }
    }
    
    */
    
    for (int i = 0; i < chains.size(); i++) {
        chains[i].display();
    }
    for (int i = 0; i < attackers.size(); i++) {
        attackers[i].get()->display();
    }
    
    // draw the ground
    string timeLeft = ofToString(abs(int(ofGetElapsedTimef() - gameTimer)));
    string timeTil = ofToString(abs(int(ofGetElapsedTimef() - timeUntilNextGame)));
    if (gameStarted) {
        if (ofGetElapsedTimef() < gameTimer - 28) {
            ofSetColor(0);
            timerText.drawStringCentered("destroy!", ofGetWidth()/2-2, ofGetHeight()/2-2);
            ofSetColor(255);
            timerText.drawStringCentered("destroy!", ofGetWidth()/2, ofGetHeight()/2);
            
        }
//        ofSetColor(0);
//        timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 18, timerText.getLineHeight() + 18);
//        ofSetColor(255);
//        timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 20, timerText.getLineHeight() + 20);

    }
    else {
        //team winner? display all team member names?
        if (winnerID >= 0 && players.size() > 0) {
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
    m.clear();
    m.setAddress("set");
    m.addStringArg("state");
    m.addIntArg(state);
    oscSender.sendMessage(m);
    m.clear();

    //set the playing variable as well
    if (state == GAME_STATE_PLAYING) {
        m.setAddress("set");
        m.addStringArg("playing");
        m.addIntArg(1);
        oscSender.sendMessage(m);
    }
    
}

void testApp::broadcastScores() {
    broadcastState(GAME_STATE_SHOW_SCORE);
    for (int i = 0; i < players.size(); i++) {
        sendScore(players[i].playerIp, players[i].score);
    }

}

void testApp::sendInstructions(string ip, string text) {
    ofxOscMessage m;
    m.setAddress("set");
    m.addStringArg("instructions");
    m.addStringArg(text);
    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}
void testApp::sendScore(string ip, int score) {
    ofxOscMessage m;
    m.setAddress("set");
    m.addStringArg("score");
    m.addIntArg(score);
    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::sendControl(string ip, int control) {
    ofxOscMessage m;
    m.setAddress("set");
    m.addStringArg("control");
    m.addIntArg(control);

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
        sendControl(playerIPs[i], control);
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
void testApp::resetConnections() {
    for (int i = 0; i < players.size(); i++) {
        players[i].connected = false;
        ofxOscMessage m;
        m.setAddress("/reset");
        m.addIntArg(0);
        oscSender.setup(players[i].playerIp, CLIENT_PORT);
        m.setRemoteEndpoint(players[i].playerIp, CLIENT_PORT);
        oscSender.sendMessage(m);
    }
}

//--------------------------------------------------------------
void testApp::joinGame(string ip) {
    ofxOscMessage msg;

    //set player ID
    msg.setAddress("set");
    msg.addStringArg("id");
    msg.addIntArg(players.size()-1);
    oscSender.setup(ip, CLIENT_PORT);
    msg.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(msg);
    msg.clear();
    //what image set to use
    msg.setAddress("set");
    msg.addStringArg("images");
    msg.addIntArg(IMAGE_SET_HUMANS);
    oscSender.sendMessage(msg);
    msg.clear();
    //what game we're playing
    msg.setAddress("set");
    msg.addStringArg("game");
    msg.addStringArg("surface race");
    oscSender.sendMessage(msg);
    msg.clear();
/*
    //give some instructions while they wait
    msg.setAddress("set");
    msg.addStringArg("instructions");
    msg.addStringArg("when the game starts\ndrag your avatar around to\nfeed on the plankton");
    oscSender.sendMessage(msg);
    msg.clear();

*/
    //now wait until we're ready to play
    sendState(ip, GAME_STATE_WAITING);
 }

//--------------------------------------------------------------
void testApp::startGame() {
    //create all players

    for (int i = 0; i < chains.size(); i++) {
        for (int j = 0; j < chains[i].pods.size(); j++) {
            chains[i].pods[j]->destroy();
        }
    }

    chains.clear();

//    ofAddListener(box2d.contactStartEvents, this, &testApp::attackerContact);
    
    cout << "Starting game, everyone should have tilt controls" << endl;
    
    //how many vehicles?
    if (players.size() < NUMBER_OF_LANES) {
        //only make a few vehicles
        for (int i = 0; i < players.size(); i++) {
            //vehicle per player
            Chain vehicle;
            vehicle.create(30 + i * (ofGetWidth()/NUMBER_OF_LANES), ofGetHeight(), &box2d);
            vehicle.image = &playerImages[i];
          //  vehicle.passengerImage = &playerImages[i];
            chains.push_back(vehicle);
        }
    }
    else {
        for (int i = 0; i < NUMBER_OF_LANES; i++) {
            Chain vehicle;
            vehicle.image = &playerImages[i];
            vehicle.create(30 + i * (ofGetWidth()/NUMBER_OF_LANES), ofGetHeight(), &box2d);
         //   vehicle.passengerImage = &playerImages[i];
            chains.push_back(vehicle);
        }
    }
    
    for (int i = 0; i < players.size(); i++) {
        chains[i%NUMBER_OF_LANES].newPod(players[i].playerIp);
        //notify player of their pod
    }

    
    
    
   
    gameStarted = true;
    gameState = GAME_STATE_PLAYING;
    broadcastState(GAME_STATE_PLAYING);
    gameTimer = ofGetElapsedTimef() + 30;
    for (int i = 0; i < players.size(); i++) {
        players[i].resetScore();
    }
    ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
    ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if (key == ' ') {
        startGame();
    }
    if (key == 'r') {
        winnerID = -1;
        resetConnections();
        imageCounter = 0;
    }
    if (key == 'k') {
        cout << "removing dead players" << endl;
        ofRemove(players,removeDeadPlayer);
    }
    
    if (key == 'n') {
        cout << "new player created" << endl;
        newPlayer("zombie" + ofToString(imageCounter), ofToString(ofRandom(5)));
    }
 }

void testApp::newAttacker() {
    ofPtr<Attacker> a = ofPtr<Attacker>(new Attacker);
    a.get()->setPhysics(5, 1, 1);
    cout << "Creating new attacker" << endl;
    float aSize = ofRandom(.2, .3);
    int planktonNumber = int(ofRandom(5));
    a.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(0, -ofGetHeight()), 50 * aSize, 50 * aSize);
    
    a.get()->setVelocity(0, ofRandom(1,4));
    //f.get()->image = &plankton[planktonNumber];
    
    a.get()->setData(new CustomData());
    a.get()->setupCustom(attackers.size());
    attackers.push_back(a);
    
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



//--------------------------------------------------------------
void testApp::contactStart(ofxBox2dContactArgs &e) {
    
    if(e.a != NULL && e.b != NULL) {
        CustomData *data1 = (CustomData *)e.a->GetBody()->GetUserData();
        CustomData *data2 = (CustomData *)e.b->GetBody()->GetUserData();
        if (data1 != NULL && data2 != NULL) {
            if (data1->type == data2->type) {
                //colliding against each other
                if (data1->type == TYPE_BULLET) {
                    //BULLETS COLLIDING WITH EACH OTHER
                }
                if (data1->type == TYPE_TANK) {
                    //TANK ON TANK COLLISION
                }
            }
            else {
                collectFx.play();
                int tankId;
                //two different types colliding, lets figure it out
                //if more than 2 types, needs more logic
                if (data1->type == TYPE_BULLET) {
                    //data2 is tank
                    //remove tank armor
           //         tanks[data2->id].get()->armor--;
                    tankId = data2->id;
                    data1->remove = true;
                    // newFood();
                }
                else if (data2->type == TYPE_BULLET){
             //       tanks[data1->id].get()->armor--;
                    tankId = data1->id;
                    data2->remove = true;
                }
                
                for (int i = 0; i < 3; i++) {
                    //send updated armor to tank
                    string armorLeft;
               //     for (int s = 0; s < tanks[tankId].get()->armor; s++) {
                //        armorLeft = armorLeft + "|";
                //    }
                //    sendInstructions(tanks[tankId].get()->ips[i], armorLeft + " ARMOR LEFT ");
                }
            }
        }
        else if (data2 != NULL) {
            data2->remove = true;
        }

        else if (data1 != NULL) {
            data1->remove = true;
        }
    }
    else {
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


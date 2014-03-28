#include "testApp.h"

#define NUMBER_OF_PLAYERS 100
#define NUMBER_OF_ROWS    10
#define PLAYER_SIZE       35
#define STARTING_PORT     9000
#define AMOUNT_OF_FOOD    100

#define WAITING_FOR_PLAYERS 0
#define PLAYING_GAME_1      1
#define PLAYING_GAME_2      2
#define PLAYING_GAME_3      3
#define PLAYING_GAME_4      4
#define PLAYING_GAME_5      5
#define PLAYING_GAME_6      6
#define GAME_OVER           7

#define SHOWING_WINNER      20

static bool removeShape(ofPtr<ofxBox2dBaseShape> shape) {
    CustomData *custom = (CustomData *)shape->getData();
    return custom->remove;
}

//--------------------------------------------------------------
void testApp::setup() {
	ofSetVerticalSync(true);
    ofSetFrameRate(30);
	ofSetLogLevel(OF_LOG_VERBOSE);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofBackground(13, 45, 152);
    for (int i = 0; i < 3; i++) {
        sea[i].loadImage("wave" + ofToString(i) + ".png");
    }
 
    bonjour = new ofxBonjourIp();
    bonjour->addEventListeners(this); // optional
    
    bonjour->startService();
    
    bonjour->discoverService();
    largeGameText.loadFont("joystix.ttf", 100);
    gameText.loadFont("joystix.ttf", 50);
	
    //Server Setup
    serverRecvPort = STARTING_PORT;
    serverReceiver.setup(serverRecvPort);

	box2d.init();
	box2d.setGravity(0, 0);
    box2d.enableEvents();
	box2d.createBounds();
	box2d.setFPS(30.0);
	for (int i = 0; i < 10; i++) {
        playerImages[i].loadImage(ofToString(i) + ".png");
    }
    gameState = WAITING_FOR_PLAYERS;
    logo.loadImage("nerdlab.png");
    
    mainMenu = new ofxUICanvas();
    mainMenu->addImageButton("PLAY", "play.png", false, 200, 16);

    ofAddListener(mainMenu->newGUIEvent,this,&testApp::guiEvent);
    mainMenu->setPosition(logo.width, logo.height + 50);
    debugging = true;
    mainMenu->setHeight(25);
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    if (kind == OFX_UI_WIDGET_IMAGEBUTTON) {
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if (name == "PLAY" && button->getValue()) {
            mainMenu->setVisible(false);
            gameState = PLAYING_GAME_1;
            ofRemoveListener(mainMenu->newGUIEvent, this, &testApp::guiEvent);
            broadcastFeedback("state", GAME_STATE_PLAYING);
/*
            ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
            ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);
*/
        }
        
    }

}

void testApp::exit() {
    delete mainMenu;
}
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::update() {

    //always let players join and move around for now
    box2d.update();
    //Check for incoming player messages
    while (serverReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        serverReceiver.getNextMessage(&msg);
        //JOIN GAME

        if (msg.getAddress() == "/join") {
            cout << "someone is joining" << endl;

            //PLAYER JOINING
            string incomingPlayer = msg.getRemoteIp();
            //take in their name
            string playerName = msg.getArgAsString(0);
            if (std::find(knownClients.begin(), knownClients.end(), incomingPlayer) == knownClients.end()) {

                int newPlayerId = newPlayer(playerName, incomingPlayer);
                //join a game with the drag control
                joinGame(newPlayerId, GAME_CONTROL_MOVE, GAME_STATE_PLAYING);
                sendFeedback(players[newPlayerId].get()->playerIp, "state", GAME_STATE_WAITING);

            }
            
        }
        //PLAYER UPDATES
        if (msg.getAddress() == "/move" && gameState == PLAYING_GAME_1) {
            if (msg.getNumArgs() == 3) {
                int playerNumber = msg.getArgAsInt32(0);
                if (players.size() > playerNumber) {
                    float xSpeed = msg.getArgAsFloat(1) * -.1;
                    float ySpeed = msg.getArgAsFloat(2) * -.1;
                //   assign humanoid id to player?
                // assign playerID to humanoid?
                    // players[playerNumber].avatar.get()->setVelocity(xSpeed, ySpeed);

                }
            }
        }
        
    }

    
    
    switch (gameState) {
        case WAITING_FOR_PLAYERS:
            break;
        case PLAYING_GAME_1:
            ofRemove(food, removeShape);
            if (food.size() < AMOUNT_OF_FOOD) {
                for (int i = food.size(); i < AMOUNT_OF_FOOD; i++) {
                    newFood();
                }
            }

            
            break;
        case SHOWING_WINNER:
            //
            break;
            
        default:
            break;
    }
}


//--------------------------------------------------------------
void testApp::draw() {
    for (int x = 0; x < ofGetWidth(); x += sea[0].width){
        
        sea[2].draw(x, ofGetHeight() - (sea[0].height * 1.9));
        sea[1].draw(x, ofGetHeight() - (sea[0].height * 1.5));
        sea[0].draw(x, ofGetHeight() -  sea[0].height);
        
    }
    string info = "";

    info += "Players: " + ofToString(knownClients.size()) + "\n";
    //            info += "FPS: "+ofToString(ofGetFrameRate(), 1)+"\n";
    info += "Host: " + bonjour->getDeviceHostName() + "\n";
    info += "IP: " + bonjour->getDeviceIp()+ "\n";
    info += "SSID: SecureCanes\n";
    info += "Controller for iOS: http://bit.ly/1iu6WhF\n";
    info += "Controller for Android: http://bit.ly/1gi8zj0\n";
    
    for (int i = 0; i < players.size(); i++) {
        info+= players[i].get()->name + " has joined the game\n";
    }

    //DEBUG INFO
    for (int i = 0; i < players.size(); i++) {
     //   players[i].get()->display();
    }
    
    float fontWidth = largeGameText.stringWidth(ofToString(int(gameTimer - ofGetElapsedTimef()))) + 20;

    switch (gameState) {
        case WAITING_FOR_PLAYERS:
            ofSetColor(255,255,255,200);
            logo.draw(ofGetWidth()/2-logo.width/2, 20);
            
            break;
        case PLAYING_GAME_1:
            ofSetColor(255,255,255);


            for (int i = 0; i < food.size(); i++) {
          //      food[i].get()->display();
            }
            // draw the ground
            box2d.drawGround();
            
            ofSetColor(0);
            largeGameText.drawString(ofToString( int(gameTimer - ofGetElapsedTimef())), ofGetWidth() - fontWidth, 115);
            ofSetColor(255, 255, 255);
            largeGameText.drawString(ofToString( int(gameTimer - ofGetElapsedTimef())), ofGetWidth() - fontWidth, 120);
            

            break;
        case SHOWING_WINNER:
            ofSetColor(0);
            largeGameText.drawString("Game Over", 20, ofGetHeight()/3 );
            ofSetColor(255,255,255);
            largeGameText.drawString("Game Over", 25, ofGetHeight()/3 + 5);
            
            if (players.size() > 0 && winner != -1) {
                ofSetColor(0);
                gameText.drawString(players[winner].get()->name + " wins!", 20, ofGetHeight()/3 + 100);
                ofSetColor(255, 255, 255);
                gameText.drawString(players[winner].get()->name + " wins!", 25, ofGetHeight()/3 + 105);
            }
            break;
        default:
            break;
    }
    if (debugging ) {
        ofSetHexColor(0xffffff);
        ofDrawBitmapString(info, 20, ofGetHeight()-100);
    }
}

ofColor testApp::getTeamColor(int teamId) {
    switch (teamId) {
        case 0:
            return ofColor::springGreen;
            break;
        case 1:
            return ofColor::orchid;
            break;
        case 2:
            return ofColor::blueViolet;
            break;
        case 3:
            return ofColor::turquoise;
            break;
        case 4:
            return ofColor::whiteSmoke;
            break;
        case 5:
            return ofColor::tomato;
            break;
        case 6:
            return ofColor::violet;
            break;
        case 7:
            return ofColor::mintCream;
            break;
        case 8:
            return ofColor::darkorange;
            break;
        case 9:
            return ofColor::darkMagenta;
            break;
        default:
            return ofColor::darkGrey;
            break;
    }

}


int testApp::newPlayer(string pname, string ip) {

        knownClients.push_back(ip);
        //send feedback to player
        serverSender.setup(ip, serverRecvPort + 1);
        //create player
        ofPtr<Player> p = ofPtr<Player>(new Player);
        //remember their name
  //      p.get()->name = pname;
  //      p.get()->playerId = knownClients.size() -1;
   //     p.get()->teamId = (knownClients.size() - 1) % PLAYERS_PER_TEAM;
   //     p.get()->subTeamId = (knownClients.size() - 1) % NUMBER_OF_PLAYERS;
/*
        p->avatar.get()->setPhysics(1.0, .5, 0.3);
        p->avatar.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), PLAYER_SIZE, PLAYER_SIZE);
    
        p->avatar.get()->color = getTeamColor(p.get()->teamId);
        p->avatar.get()->image = &playerImages[p.get()->subTeamId];
        p->avatar.get()->setupCustom(ip);
*/
// players.push_back(p);
        return players.size() -1;

}

void testApp::joinGame(int playerId, int control, int state) {
    ofxOscMessage msgToPlayer;
    //client will look for joined message, the argument is their player number
    msgToPlayer.setAddress("/joined");
    msgToPlayer.addIntArg(players[playerId].get()->playerId);
    msgToPlayer.addIntArg(players[playerId].get()->teamId);
    msgToPlayer.addIntArg(players[playerId].get()->subTeamId);
    msgToPlayer.addIntArg(control);
    msgToPlayer.addIntArg(state);
    msgToPlayer.setRemoteEndpoint(players[playerId].get()->playerIp, serverRecvPort + 1);
    serverSender.sendMessage(msgToPlayer);

}


//--------------------------------------------------------------
void testApp::keyPressed(int key) {
/*
	if (key == OF_KEY_DOWN) {
        players[0].get()->addForce(ofVec2f(0,4), 2);
    }
    if (key == OF_KEY_UP) {
        players[0].get()->addForce(ofVec2f(0,-4), 2);
        
    }
	if (key == OF_KEY_LEFT) {
        players[0].get()->addForce(ofVec2f(-4,0), 2);
    }
    if (key == OF_KEY_RIGHT) {
        players[0].get()->addForce(ofVec2f(4,0), 2);
        
    }
  */
	if(key == 't') ofToggleFullscreen();
    if(key == 'd') {
        debugging = !debugging;
    }
    if(key == ' ') {

        if (gameState == PLAYING_GAME_1) {
            gameState = SHOWING_WINNER;
            broadcastFeedback("state", GAME_STATE_WAITING);

        }
        else if (gameState == SHOWING_WINNER) {
            broadcastFeedback("state", GAME_STATE_WAITING);
            gameState = WAITING_FOR_PLAYERS;
            cout << "resetting scores" << endl;
            for (int i = 0; i < players.size(); i++) {
                players[i].get()->resetScore();
            }
        }
    }
}

int testApp::whoWon() {
    int highScore = -999;
    int theWinner = -1;
    for (int i = 0; i < players.size(); i++) {
        if (players[i].get()->score>highScore) {
            winner = i;
            highScore = players[i].get()->score;
        }
    }
    return theWinner;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ) {
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){
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

//--------------------------------------------------------------
void testApp::contactStart(ofxBox2dContactArgs &e) {
    cout  << "made contact" << endl;
    
    if(e.a != NULL && e.b != NULL) {
        CustomData *data1 = (CustomData *)e.a->GetBody()->GetUserData();
        CustomData *data2 = (CustomData *)e.b->GetBody()->GetUserData();
        if (data1 != NULL && data2 != NULL) {
        if (data1->type == data2->type) {
            //colliding against each other
            if (data1->type == PLAYER_TYPE) {
                //PLAYERS COLLIDING WITH EACH OTHER
            }
            if (data1->type == FOOD_TYPE) {
                //FOOD ON FOOD COLLISION
            }
        }
        else {
            //two different types colliding, lets figure it out
            //if more than 2 types, needs more logic
            if (data1->type == FOOD_TYPE) {
                //food is data1, player is data2
                data1->remove = true;
                players[data2->id].get()->score+= food[data1->id].get()->getWidth();
               // newFood();
            }
            else {
                //food is data2, player is data1
                data2->remove = true;
                players[data1->id].get()->score+= food[data2->id].get()->getWidth();
               // newFood();
            }

        }
        }
    }
}
void testApp::newFood() {
    ofPtr<Food> f = ofPtr<Food>(new Food);
    f.get()->setPhysics(.5, 1, 1);
    float fSize = ofRandom(.1, .5);
    int planktonNumber = int(ofRandom(5));
    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));
    f.get()->image = &plankton[planktonNumber];

    f.get()->setData(new CustomData());
    f.get()->setupCustom(food.size());
    food.push_back(f);

}
//--------------------------------------------------------------
void testApp::contactEnd(ofxBox2dContactArgs &e) {
    cout << "contact ended" << endl;
    if (e.a != NULL && e.b != NULL) {
//        Player *player = (Player*)e.a->GetBody()->GetUserData();
    }

}


/* TWO LAST FUNCTIONS ARE FROM CHAT SYSTEM EXAMPLE in oF Addons*/
//--------------------------------------------------------------
string testApp::getOscMsgAsString(ofxOscMessage m){
	string msg_string;
	msg_string = m.getAddress();
	msg_string += ":";
	for(int i = 0; i < m.getNumArgs(); i++){
		// get the argument type
		msg_string += " " + m.getArgTypeName(i);
		msg_string += ":";
		// display the argument - make sure we get the right type
		if(m.getArgType(i) == OFXOSC_TYPE_INT32){
			msg_string += ofToString(m.getArgAsInt32(i));
		}
		else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
			msg_string += ofToString(m.getArgAsFloat(i));
		}
		else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
			msg_string += m.getArgAsString(i);
		}
		else{
			msg_string += "unknown";
		}
	}
	return msg_string;
}
void testApp::sendFeedback(string ip, string control, int param) {
    ofxOscMessage m;
	m.setAddress("/feedback");
	m.addStringArg(control);
    m.addIntArg(param);
    serverSender.setup(ip, serverRecvPort + 1);
    m.setRemoteEndpoint(ip, serverRecvPort + 1);
    serverSender.sendMessage(m);
}

void testApp::broadcastFeedback(string control, int param) {
	//create a new OSC message
	ofxOscMessage m;
	m.setAddress("/feedback");
	m.addStringArg(control);
    m.addIntArg(param);
    
	//Send message to all known hosts
	// use another port to avoid a localhost loop
	for(unsigned int i = 0; i < knownClients.size(); i++){
		serverSender.setup(knownClients[i], serverRecvPort + 1);
		m.setRemoteEndpoint(knownClients[i], serverRecvPort + 1);
		serverSender.sendMessage(m);
		ofLogVerbose("Server broadcast message " + m.getArgAsString(0) + " to " + m.getRemoteIp()
					 + ":" + ofToString(m.getRemotePort()));
	}
    
}


void testApp::broadcastReceivedMessage(string message){
    
	//create a new OSC message
	ofxOscMessage m;
	m.setAddress("/feedback");
	m.addStringArg(message);
    
	//Send message to all known hosts
	// use another port to avoid a localhost loop
	for(unsigned int i = 0; i < knownClients.size(); i++){
		serverSender.setup(knownClients[i], serverRecvPort + 1);
		m.setRemoteEndpoint(knownClients[i], serverRecvPort + 1);
		serverSender.sendMessage(m);
		ofLogVerbose("Server broadcast message " + m.getArgAsString(0) + " to " + m.getRemoteIp()
					 + ":" + ofToString(m.getRemotePort()));
	}
}


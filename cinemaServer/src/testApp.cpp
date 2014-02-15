#include "testApp.h"

#define NUMBER_OF_PLAYERS 100
#define NUMBER_OF_ROWS    10
#define PLAYER_SIZE       35
#define STARTING_PORT     9000
#define AMOUNT_OF_FOOD    50

static bool removeShape(ofPtr<ofxBox2dBaseShape> shape) {
    //CustomData *custom = (CustomData *)shape->GetBody()->GetUserData();
    CustomData *custom = (CustomData *)shape->getData();
    return custom->remove;
}

//--------------------------------------------------------------
void testApp::setup() {
	ofSetVerticalSync(true);
    ofSetFrameRate(30);
	ofSetLogLevel(OF_LOG_VERBOSE);
    ofBackground(0, 0, 255);

    largeGameText.loadFont("mono.ttf", 100);
	
    //Server Setup
    serverRecvPort = STARTING_PORT;
    serverReceiver.setup(serverRecvPort);

	box2d.init();
	box2d.setGravity(0, 0);
    box2d.enableEvents();
	box2d.createBounds();
	box2d.setFPS(30.0);
//	box2d.registerGrabbing();
	for (int i = 0; i < 5; i++) {
        playerImages[i].loadImage("star.png");
    }
//CREATE food
    
    for (int i = 0; i < AMOUNT_OF_FOOD; i++) {
        newFood();
    }

    // register the listener so that we get the events
	ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
	ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);
    newPlayer("clay");
}

//--------------------------------------------------------------
void testApp::update() {
    ofRemove(food, removeShape);
    if (food.size() < AMOUNT_OF_FOOD) {
        for (int i = food.size(); i < AMOUNT_OF_FOOD; i++) {
            newFood();
        }
    }
	box2d.update();
    //Check for incoming player messages
    while (serverReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        serverReceiver.getNextMessage(&msg);
        ofLogVerbose("Server Received Message: " + getOscMsgAsString(msg) + " from " + msg.getRemoteIp());
        //JOIN GAME
        if (msg.getAddress() == "/join") {
            //PLAYER JOINING
            string incomingPlayer = msg.getRemoteIp();
            //take in their name
            string playerName = msg.getArgAsString(0);
            //is this a new player?
            if (std::find(knownClients.begin(), knownClients.end(), incomingPlayer) == knownClients.end()) {
                knownClients.push_back(incomingPlayer);
                //send feedback to player
                serverSender.setup(incomingPlayer, serverRecvPort + 1);
                //create player
                ofPtr<Player> p = ofPtr<Player>(new Player);
                //remember their name
                p.get()->name = playerName;
                //delegate their number based on the size of known clients minus 1 (counting from zero)
                p.get()->playerId = knownClients.size() - 1;
                p.get()->setPhysics(1.0, .5, 0.3);
                p.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), PLAYER_SIZE, PLAYER_SIZE);
                p.get()->color = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
                p.get()->image = &playerImages[0];
                p.get()->setupCustom(knownClients.size() -1);

                ofxOscMessage msgToPlayer;
                //client will look for joined message, the argument is their player number
                msgToPlayer.setAddress("/joined");
                msgToPlayer.addIntArg(p.get()->playerId);
                msgToPlayer.addIntArg(p.get()->teamId);
                msgToPlayer.addIntArg(p.get()->subTeamId);
                //define shape ids
                //            msgToPlayer.addIntArg(p.get()->playerId);
                msgToPlayer.setRemoteEndpoint(incomingPlayer, serverRecvPort + 1);
                serverSender.sendMessage(msgToPlayer);

                players.push_back(p);
                
                ofLogVerbose("Sent message " + msgToPlayer.getArgAsString(0) + " to " + msgToPlayer.getRemoteIp()
                             + ":" + ofToString(msgToPlayer.getRemotePort()));

            }

        }
        //PLAYER MOVEMENT
        if (msg.getAddress() == "/move") {
            if (msg.getNumArgs() == 3) {
                int playerNumber = msg.getArgAsInt32(0);
                float xSpeed = msg.getArgAsFloat(1) * -.1;
                float ySpeed = msg.getArgAsFloat(2) * -.1;
                players[playerNumber].get()->setVelocity(xSpeed, ySpeed);
                cout << "Changing " << playerNumber << " to " << xSpeed << ", " << ySpeed<< endl;
                
            }
        }
    }
}


//--------------------------------------------------------------
void testApp::draw() {

    for (int i = 0; i < players.size(); i++) {
        players[i].get()->display();
    }
    for (int i = 0; i < food.size(); i++) {
        food[i].get()->display();
    }
	// draw the ground
	box2d.drawGround();
	
	
//DEBUG INFO
	string info = "";
    info += "Players: " + ofToString(knownClients.size()) + "\n";
	info += "FPS: "+ofToString(ofGetFrameRate(), 1)+"\n";
    for (int i = 0; i < players.size(); i++) {
        info+= players[i].get()->name + ": " + ofToString(players[i].get()->score) + "\n";
    }
	ofSetHexColor(0xffffff);
	ofDrawBitmapString(info, 30, 30);
}

void testApp::newPlayer(string pname) {
    ofPtr<Player> p = ofPtr<Player>(new Player);
    //remember their name
    p.get()->name = pname;
    //delegate their number based on the size of known clients minus 1 (counting from zero)
    p.get()->playerId = players.size();
    p.get()->setPhysics(1.0, .5, 0.3);
    p.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), PLAYER_SIZE, PLAYER_SIZE);
    p.get()->color = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
    p.get()->image = &playerImages[0];
    p.get()->setupCustom(players.size());

    players.push_back(p);

}
//--------------------------------------------------------------
void testApp::keyPressed(int key) {
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
    
	if(key == 't') ofToggleFullscreen();
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
                players[data2->id].get()->score+= food[data1->id].get()->getRadius();
               // newFood();
            }
            else {
                //food is data2, player is data1
                data2->remove = true;
                players[data1->id].get()->score+= food[data2->id].get()->getRadius();
               // newFood();
            }

        }
        }
    }
}
void testApp::newFood() {
    ofPtr<Food> f = ofPtr<Food>(new Food);
    f.get()->setPhysics(.5, 1, 1);

    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), ofRandom(10,20));
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));

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


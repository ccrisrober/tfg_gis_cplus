// Copyright (c) 2015, maldicion069 (Cristian Rodríguez) <ccrisrober@gmail.con>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.package com.example

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/Network.hpp>
#include "Jzon.h"
#include "TileMap.h"
#include <sstream>
#include "AnimatedSprite.h"
#include <iostream>
#include <math.h>
#include "TCPEvents.h"
#include <set>
#include "PlayerSprite.h"
#include <fstream>      // std::ifstream
#include "Key.h"

// TODO: Liberar memoria str 21/01/20015 20:03
// TODO : SI escondo un sprite, ¿se puede realizar una acción sobre él? (yo creo que sí) 22/01/2015 19:35
 

// Problemas:
//	Go necesita que los atributos del JSON empiecen en mayúscula (20/01/2015 - 12:39
//

// CONST
#define UP 0
#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define FIGHT 4
//#define PORT 3337
#define MAXBUFFINIT 6144
#define SEMIBUFF 512
#define TILESETNAME "tileset.png"
#define WIDTH 768
#define HEIGHT 576
#define DEFAULT_OBJECTS_TEXT "Objects: "
#define DEFAULT_HEADER_TEXT "Soltar con P + Num(x)"
#define MAX_OPACITY 15

const float sendtoServerTime = 1 / 50;

// FIGHTSTATES
enum FightStates { NOT_BATTLE, WAITFORINIT, RECEIVE_INIT, SEND_ACK, INIT_FIGHT, INIT_FIGHT_BLACKWINDOW, INIT_FIGHT_ROLLDICE, DIE_PLAYER, FINISH, QUIT };

// GLOBAL VARS
struct TGlobalVars {
	int id;
	int id_fight;
	FightStates fightState;
	float zoom;
	bool battle;
	sf::View cam;
	sf::View origCam;

	bool noKeyWasPressed;
	bool choca;
	float speed;
	TileMap map;

	std::set<int> collisionsBlocks;

	//
	std::set<int> objsServerWait;
	int fightServerWait;

	TGlobalVars() {
		fightServerWait = -1;
	}
};


TGlobalVars globalVars;

Animation walkingAnimationDown, walkingAnimationLeft, walkingAnimationRight, walkingAnimationUp;
Animation* currentAnimation;
sf::Texture texture, textureSwim, textureEven, textureOdd, texture_fight;
std::map<int, PlayerSprite*> players;
sf::Keyboard::Key keyboard[5];
std::map<int, Key*> keyObjects;
std::map<int, Key*> userObjects;


std::size_t received;
std::string str;
Jzon::Parser parser;
char buffer_[SEMIBUFF];



TCP::ClientToServer::TMove tMove_toServer;
TCP::ClientToServer::TDiePlayer tDiePlayer_toServer;
TCP::ClientToServer::TExit tExit_toServer;
TCP::ClientToServer::TFreeObject tFreeObj_toServer;
TCP::ClientToServer::TGetObject tGetObj_toServer;
TCP::ClientToServer::TInitWithName tGetInitWName_toServer;
TCP::ClientToServer::TOkInitFight tOkInitFight_toServer;
TCP::ClientToServer::TSendAckInitFight tSendAckInitFight_toServer;
TCP::ClientToServer::TInitBattle tInitFight_toServer;

TCP::ServerToClient::TFinishBattle tFinishBattle_toClient;
TCP::ServerToClient::TMove tMove_toClient;
TCP::ServerToClient::TNew tNew_toClient;
TCP::ServerToClient::TExit tExit_toClient;
TCP::ServerToClient::THide tHide_toClient;
TCP::ServerToClient::TRemObject tRemObj_toClient;
TCP::ServerToClient::TAddObj tAddObj_toClient;
TCP::ServerToClient::TGetObjFromServer tGetObjFromServer_toClient;
TCP::ServerToClient::TLiberateObj tLiberateFromServer_toClient;
TCP::ServerToClient::TGetInitFight tGetInitFight_toClient;
TCP::ServerToClient::TOkInitFight tOkInitFight_toClient;


void sendExitClient(sf::TcpSocket& socket) {
	std::string message = tExit_toServer.generate();
	socket.send(message.c_str(), message.size());

	socket.disconnect();
}

void fixColor5(float& v, int color, float speed) {
	if (color == 5) {
		v += speed / 2;
	}
	else {
		v += speed;
	}
}

std::vector<int> split_(std::string &s, char delim)
{
	std::vector<int> elems;
	std::string item;

	// use stdlib to tokenize the string
	std::stringstream ss(s);
	while (std::getline(ss, item, delim))
	if (!item.empty())
		elems.push_back(std::stoi(item));

	return elems;
}

bool loadTexture(sf::Texture& texture, std::string name) {
	if (!texture.loadFromFile(name)) {
		std::cout << "Failed to load  " << name << " spritesheet!" << std::endl;
		return false;
	}
	return true;
}

void addKey(int id, std::string color, float pX, float pY) {
	Key* k = 0;
	if (color == "Red")
		k = new Key(Key::TKey::Red, id, sf::Vector2f(pX, pY));
	else if (color == "Green")
		k = new Key(Key::TKey::Green, id, sf::Vector2f(pX, pY));
	else if (color == "Yellow")
		k = new Key(Key::TKey::Yellow, id, sf::Vector2f(pX, pY));
	else if (color == "Blue")
		k = new Key(Key::TKey::Blue, id, sf::Vector2f(pX, pY));
	if (k != 0)
		keyObjects[id] = k;
}

// TODO: Esto debe ir en la lectura de datos del socket!!
void releaseObject(int id_obj, AnimatedSprite& animatedSprite, std::string& objectFooterString, sf::Text& objectFooterText, sf::TcpSocket& socket) {

	std::string message = tFreeObj_toServer.generate(*keyObjects[id_obj], globalVars.id);
	message += "\n";
	socket.send(message.c_str(), message.size());

	memset(buffer_, '\0', SEMIBUFF);
	sf::Socket::Status status = socket.receive(buffer_, SEMIBUFF, received);
	if (status != sf::Socket::Done) {
		while (socket.receive(buffer_, SEMIBUFF, received) == sf::Socket::NotReady) {
			//std::cout << received << std::endl;
		}
	}
	std::string sstr = buffer_;
	sstr = sstr.substr(0, str.find_last_of("}"));
	//std::cout << sstr << std::endl;

	Jzon::Node clientJson = parser.parseString(sstr.substr(0, sstr.find_last_of("}") + 1));
	if (!clientJson.isValid()) {
		std::cout << "Error :" << parser.getError() << std::endl;
	}
	tLiberateFromServer_toClient.load(clientJson);

	if (tLiberateFromServer_toClient.ok == 1) {
		keyObjects[id_obj]->setPosition(animatedSprite.getPosition());
		keyObjects[id_obj]->setVisible(true);
		userObjects.erase(id_obj);
		std::stringstream sstm;
		sstm << DEFAULT_OBJECTS_TEXT;
		for (auto& kv : userObjects) {
			if (kv.second != NULL)
				sstm << kv.first << ", ";
		}
		objectFooterString = sstm.str();
		if (userObjects.size() > 0) {
			objectFooterString = objectFooterString.substr(0, objectFooterString.length() - 2);
		}
		objectFooterText.setString(objectFooterString);
	}
}

void getMap(Jzon::Node& node, std::vector<int>& map, int& width, int& height, float& posX, float& posY) {
	Jzon::Node mapx = node.get("Map");

	Jzon::Node mapFields = mapx.get("mapFields");

	Jzon::Node keys_ = mapx.get("keyObjects");

	Key::initialize();

	for (auto it = keys_.begin(); it != keys_.end(); it++) {
		std::cout << "Una llave" << std::endl;
		std::string color = (*it).second.get("color").toString();
		std::cout << color << std::endl;
		float pX = (*it).second.get("posX").toFloat();
		float pY = (*it).second.get("posY").toFloat();
		int id = (*it).second.get("id").toInt();
		addKey(id, color, pX, pY);		
	}

	map = split_(mapFields.toString(), ',');
	width = mapx.get("width").toInt(0);
	height = mapx.get("height").toInt(0);

	globalVars.id = node.get("Id").toInt(0);
	posX = node.get("X").toFloat();	// TODO :Arreglar esto que debería ser PosX y si no da error de la cámara D:
	posY = node.get("Y").toFloat();

	// Guardo las posiciones de los usuarios conectados con anterioridad
	Jzon::Node users_ = node.get("Users");
	for (auto it = users_.begin(); it != users_.end(); it++) {
		int _id_ = (*it).second.get("id").toInt();
		if (_id_ != globalVars.id) {
			std::cout << "Un user" << std::endl;
			float _x_ = (*it).second.get("posX").toFloat();
			float _y_ = (*it).second.get("posY").toFloat();

			PlayerSprite *ps = new PlayerSprite(_id_);
			ps->setPosition(_x_, _y_);
			players.insert(std::pair<int, PlayerSprite*>(_id_, ps));
		}
	}

	// Guardo los objetos que tenga el usuario en su sesión
	Jzon::Node objects_ = node.get("Objects");
	for (auto it = objects_.begin(); it != objects_.end(); it++) {
		int _id_ = (*it).second.get("id").toInt();
		float _x_ = (*it).second.get("posX").toFloat();
		float _y_ = (*it).second.get("posY").toFloat();
		Key* k = new Key(Key::intToEnum(_id_), _id_, sf::Vector2f(_x_, _y_));
		k->setDisponible(true);
		k->setVisible(false);
		userObjects[_id_] = k;
		keyObjects[_id_] = k;
	}
}

void setAnimations(Animation& walkingAnimationDown, Animation& walkingAnimationLeft, Animation& walkingAnimationRight, Animation& walkingAnimationUp, sf::Texture& texture) {
	walkingAnimationDown.setSpriteSheet(texture);
	walkingAnimationDown.addFrame(sf::IntRect(64, 0, 64, 64));
	walkingAnimationDown.addFrame(sf::IntRect(128, 0, 64, 64));
	walkingAnimationDown.addFrame(sf::IntRect(64, 0, 64, 64));
	walkingAnimationDown.addFrame(sf::IntRect(0, 0, 64, 64));

	walkingAnimationLeft.setSpriteSheet(texture);
	walkingAnimationLeft.addFrame(sf::IntRect(64, 64, 64, 64));
	walkingAnimationLeft.addFrame(sf::IntRect(128, 64, 64, 64));
	walkingAnimationLeft.addFrame(sf::IntRect(64, 64, 64, 64));
	walkingAnimationLeft.addFrame(sf::IntRect(0, 64, 64, 64));

	walkingAnimationRight.setSpriteSheet(texture);
	walkingAnimationRight.addFrame(sf::IntRect(64, 128, 64, 64));
	walkingAnimationRight.addFrame(sf::IntRect(128, 128, 64, 64));
	walkingAnimationRight.addFrame(sf::IntRect(64, 128, 64, 64));
	walkingAnimationRight.addFrame(sf::IntRect(0, 128, 64, 64));

	walkingAnimationUp.setSpriteSheet(texture);
	walkingAnimationUp.addFrame(sf::IntRect(64, 192, 64, 64));
	walkingAnimationUp.addFrame(sf::IntRect(128, 192, 64, 64));
	walkingAnimationUp.addFrame(sf::IntRect(64, 192, 64, 64));;
	walkingAnimationUp.addFrame(sf::IntRect(0, 192, 64, 64));
}

void fixBufferTCP(char buffer[], std::string& str) {
	str = buffer;
	str = str.substr(0, str.find_last_of("}") + 1);
	std::cout << str << std::endl;
}

int getPort() {
	std::ifstream infile;

	infile.open("test.txt");

	std::string sLine;

	while (!infile.eof()) {
		infile >> sLine;
		break;
	}

	infile.close();

	std::istringstream buf(sLine);

	int port;
	buf >> port;
	std::cout << port << std::endl;
	return 8085; // port; // 8089; // TODO :HArdcoded port;
}

sf::Texture defineKeyboard(int id) {
	if (id % 2 == 0) {
		keyboard[UP] = sf::Keyboard::Up;
		keyboard[LEFT] = sf::Keyboard::Left;
		keyboard[RIGHT] = sf::Keyboard::Right;
		keyboard[DOWN] = sf::Keyboard::Down;
		keyboard[FIGHT] = sf::Keyboard::RControl;
		std::cout << "Configuración normal" << std::endl;
		return textureEven;
	}
	else {
		keyboard[UP] = sf::Keyboard::W;
		keyboard[LEFT] = sf::Keyboard::A;
		keyboard[RIGHT] = sf::Keyboard::D;
		keyboard[DOWN] = sf::Keyboard::S;
		keyboard[FIGHT] = sf::Keyboard::Z;
		std::cout << "Configuración WADS" << std::endl;
		return textureOdd;
	}
}

bool initSocket(sf::TcpSocket& socket) {
	sf::IpAddress ip = "127.0.0.1";

	// Read port from file
	sf::Socket::Status status = socket.connect(ip, getPort());	//PORT);
	return status == sf::Socket::Done;
}

bool initGame(sf::TcpSocket& socket, float& posX, float& posY, char* buffer, 
	std::size_t& received, std::string& str, Jzon::Parser& parser, 
	TileMap& map) {

	std::string act = "";
	Jzon::Node node = Jzon::null();
	while (act != "sendMap") {

		sf::Socket::Status status = socket.receive(buffer, MAXBUFFINIT, received);

		if (status != sf::Socket::Done)
			while (socket.receive(buffer, MAXBUFFINIT, received) == sf::Socket::NotReady) {
				std::cout << received << std::endl;
			}
		std::cout << "OK" << std::endl;

		fixBufferTCP(buffer, str);

		//std::cout << "Received " << str << std::endl;

		node = parser.parseString(str);
		if (!node.isValid()) {
			std::cout << "Error" << std::endl;
		}
		act = node.get("Action").toString();
	}
	std::vector<int> vv;

	posX = 0;
	posY = 0;
	int width_json, height_json;

	getMap(node, vv, width_json, height_json, posX, posY);

	std::cout << "ID : " << globalVars.id << std::endl;

	str[0] = '\0';

	// create the tilemap from the level definition
	if (!map.load(TILESETNAME, sf::Vector2u(64, 64), vv, width_json, height_json)) {
		return false;
	}
	return true;
}

void updateZoom(AnimatedSprite& animatedSprite) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
		if (globalVars.zoom < 15) {
			globalVars.cam.zoom(1.05f);
			globalVars.zoom += 1.05f;
			std::cout << globalVars.zoom << std::endl;
		}
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
		if (globalVars.zoom > -15) {
			globalVars.cam.zoom(0.95f);
			globalVars.zoom -= 0.95f;
			std::cout << globalVars.zoom << std::endl;
		}
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
		globalVars.cam = globalVars.origCam;
		globalVars.cam.setCenter(animatedSprite.getPosition());
		globalVars.zoom = 0;
	}
}

void fightEvents(sf::RenderWindow& window, sf::TcpSocket& socket) {

	/*sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
	}

	if (socket.receive(buffer_, SEMIBUFF, received) == sf::Socket::Done) {
		std::string sstr = buffer_;
		sstr = sstr.substr(0, str.find_last_of("}"));
		//std::cout << sstr << std::endl;

		Jzon::Node clientJson = parser.parseString(sstr.substr(0, sstr.find_last_of("}") + 1));
		if (!clientJson.isValid()) {
			std::cout << "Error :" << parser.getError() << std::endl;
		}

		std::string action = clientJson.get("Action").toString();

		std::cout << action << std::endl;

		if (action == "move") {
		}
	}
			memset(buffer_, '\0', SEMIBUFF);
	switch (globalVars.fightState)
	{
		// Caso 1: Muestro cartel
		//Durante 3 segundos muestro un cartel de inicio de batalla por pantalla variando la opacidad hasta 0
		
	case FightStates::INIT:
		break;
		// Caso 2: Muestro un dado hasta que el usuario pulsa X
	case FightStates::DIE_RANDOM:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
			globalVars.fightState = FightStates::DIE_PLAYER;
		}
		break;
	case FightStates::DIE_PLAYER:
		break;
	case FightStates::FINISH:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
			globalVars.fightState = FightStates::QUIT;
		}
		break;
	case FightStates::QUIT:
		break;
	}*/
}

sf::Vector2f movePlayer(AnimatedSprite& animatedSprite) {
	sf::Vector2f movement(0.f, 0.f);

	if (sf::Keyboard::isKeyPressed(keyboard[UP])) {
		int color = globalVars.map.types[round(animatedSprite.getGlobalBounds().left / 64)][round((animatedSprite.getGlobalBounds().top - 32) / 64)];
		currentAnimation = &walkingAnimationUp;
		if (color == 7 && userObjects[Key::enumToInt(Key::TKey::Red)] == NULL) {
			globalVars.choca = true;
		}
		else if (globalVars.collisionsBlocks.find(color) == globalVars.collisionsBlocks.end()) {
			color = globalVars.map.types[round(animatedSprite.getGlobalBounds().left / 64)][round(animatedSprite.getGlobalBounds().top / 64 + 0.5)];
			fixColor5(movement.y, color, -globalVars.speed);
			globalVars.noKeyWasPressed = false;
			animatedSprite.setTexture(color == 8 ? &textureSwim : &texture);
		}
		else {
			globalVars.choca = true;
		}
	}
	else if (sf::Keyboard::isKeyPressed(keyboard[DOWN])) {
		int color = globalVars.map.types[round(animatedSprite.getGlobalBounds().left / 64)][ceil((animatedSprite.getGlobalBounds().top) / 64)];
		currentAnimation = &walkingAnimationDown;
		if (color == 7 && userObjects[Key::enumToInt(Key::TKey::Red)] == NULL) {
			globalVars.choca = true;
		}
		else if (globalVars.collisionsBlocks.find(color) == globalVars.collisionsBlocks.end()) {
			color = globalVars.map.types[round(animatedSprite.getGlobalBounds().left / 64)][round(animatedSprite.getGlobalBounds().top / 64 + 0.5)];
			fixColor5(movement.y, color, +globalVars.speed);
			globalVars.noKeyWasPressed = false;
			animatedSprite.setTexture(color == 8 ? &textureSwim : &texture);
		}
		else {
			globalVars.choca = true;
		}
	}
	else if (sf::Keyboard::isKeyPressed(keyboard[LEFT])) {
		int color = globalVars.map.types[round((animatedSprite.getGlobalBounds().left - 32) / 64)][round(animatedSprite.getGlobalBounds().top / 64)];
		currentAnimation = &walkingAnimationLeft;
		if (color == 7 && userObjects[Key::enumToInt(Key::TKey::Red)] == NULL) {
			globalVars.choca = true;
		}
		else if (globalVars.collisionsBlocks.find(color) == globalVars.collisionsBlocks.end()) {
			color = globalVars.map.types[round(animatedSprite.getGlobalBounds().left / 64)][round(animatedSprite.getGlobalBounds().top / 64)];
			fixColor5(movement.x, color, -globalVars.speed);
			globalVars.noKeyWasPressed = false;
			animatedSprite.setTexture(color == 8 ? &textureSwim : &texture);
		}
		else {
			globalVars.choca = true;
		}
	}
	else if (sf::Keyboard::isKeyPressed(keyboard[RIGHT])) {
		int color = globalVars.map.types[ceil((animatedSprite.getGlobalBounds().left + 1) / 64)][round(animatedSprite.getGlobalBounds().top / 64)];
		currentAnimation = &walkingAnimationRight;
		if (color == 7 && userObjects[Key::enumToInt(Key::TKey::Red)] == NULL) {
			globalVars.choca = true;
		}
		else if (globalVars.collisionsBlocks.find(color) == globalVars.collisionsBlocks.end()) {
			color = globalVars.map.types[round(animatedSprite.getGlobalBounds().left / 64)][round(animatedSprite.getGlobalBounds().top / 64)];
			fixColor5(movement.x, color, +globalVars.speed);
			globalVars.noKeyWasPressed = false;
			animatedSprite.setTexture(color == 8 ? &textureSwim : &texture);
		}
		else {
			globalVars.choca = true;
		}
	}
	return movement;
}

int main(int argc, char* argv[]) {

	std::cout << "TCP style" << std::endl;

	char buffer[MAXBUFFINIT];
	sf::TcpSocket socket;
	bool ok = initSocket(socket);

	if (!ok) {
		std::cout << "NO se ha podido conectar con el servidor" << std::endl;
		char a;
		std::cin >> a;
		return -1;
	}

	globalVars.id_fight = -1;
	globalVars.fightState = FightStates::NOT_BATTLE;
	globalVars.zoom = 1.f;
	globalVars.battle = false;

	std::cout << "Ingresa tu nombre: ";

	std::string name;
	if (argc > 1) {
		name = argv[1];
	}
	else {
		std::cin >> name;
	}

	std::cout << "Cargando ..." << std::endl;

	float posX, posY;

	// load textures
	loadTexture(textureEven, "Player.png");
	loadTexture(textureOdd, "Player2.png");
	loadTexture(textureSwim, "PlayerSwim.png");
	loadTexture(texture_fight, "fight.png");
	sf::Font font;
	font.loadFromFile("arial.ttf");

	int vDadoInt = 0;



	std::string message = tGetInitWName_toServer.generate(name);
	message += "\n";
	socket.send(message.c_str(), message.size());


	initGame(socket, posX, posY, buffer, received, str, parser, globalVars.map);

	
	// Asigno teclas según si es par o impar
	texture = defineKeyboard(globalVars.id);

	AnimatedSprite animatedSprite = AnimatedSprite(globalVars.id, sf::seconds(0.2), true, false);	// set up AnimatedSprite

	// set up the animations for all four directions (set spritesheet and push frames)
	setAnimations(walkingAnimationDown, walkingAnimationLeft, walkingAnimationRight, walkingAnimationUp, texture);

	currentAnimation = &walkingAnimationDown;

	// set up AnimatedSprite
	animatedSprite.setPosition(sf::Vector2f(posX, posY));

	sf::FloatRect finalTile = globalVars.map.getFinalTile();

	sf::Clock frameClock;
	sf::Clock frameClock2;

	globalVars.speed = 80.f;
	globalVars.noKeyWasPressed = true;
	globalVars.choca = false;

	// setup window

	sf::RenderWindow window (sf::VideoMode(WIDTH, HEIGHT), "Client Game");
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);

	globalVars.cam = window.getDefaultView();
	globalVars.origCam = window.getDefaultView();
	globalVars.cam.setCenter(animatedSprite.getPosition());
	window.setView(globalVars.cam);

	sf::Text dieRollLabel = sf::Text("", font);

	bool pulsaY = false;
	bool goY = false;
	bool pulsaP = false;
	bool goP = false;

	socket.setBlocking(false);

	sf::Sprite fight_bg(texture_fight);

	globalVars.collisionsBlocks.insert(1);
	globalVars.collisionsBlocks.insert(4);

	// Asigno identificador al AnimatedSprite
	animatedSprite.m_id = globalVars.id;

	int counterBattleInit = 0;
	int titleBattleInitOpacity = MAX_OPACITY;

	int collisionKey = -1;
	int collisionEnemy = -1;

	bool finishGame = false;

	sf::Vector2f diff(animatedSprite.getPosition() - sf::Vector2f(320, 320));

	sf::RectangleShape objectFooterBg = sf::RectangleShape(sf::Vector2f(WIDTH, 75));
	objectFooterBg.setFillColor(sf::Color::White);
	objectFooterBg.move(-64, HEIGHT);
	objectFooterBg.move(diff);

	std::string objectFooterString = DEFAULT_OBJECTS_TEXT;

	sf::Text objectFooterText = sf::Text("Objects: ", font);
	objectFooterText.setColor(sf::Color::Black);
	objectFooterText.move(-58, HEIGHT);
	objectFooterText.move(diff);
	objectFooterText.setScale(sf::Vector2f(0.75, 0.75));

	sf::RectangleShape objectHeaderBg = sf::RectangleShape(sf::Vector2f(WIDTH/2 - 90, 75));
	sf::Color whiteAlpha150 = sf::Color::White;
	objectHeaderBg.setFillColor(whiteAlpha150);
	objectHeaderBg.move(WIDTH / 2 + 80, 0);
	objectHeaderBg.move(diff);

	sf::Text objectHeaderText = sf::Text(DEFAULT_HEADER_TEXT, font);
	objectHeaderText.setColor(sf::Color::Blue);
	objectHeaderText.move(WIDTH / 2 + 90, 40);
	objectHeaderText.move(diff);
	objectHeaderText.setScale(sf::Vector2f(0.75, 0.75));
	
	sf::Vector2f pos;

	while (window.isOpen()) {
		if (globalVars.battle) {
			fightEvents(window, socket);
		}
		else {
			globalVars.fightState = FightStates::NOT_BATTLE;	// Reset fight state
			//std::cout << buffer_ << std::endl;
			if (socket.receive(buffer_, SEMIBUFF, received) == sf::Socket::Done) {
				std::string sstr = buffer_;
				sstr = sstr.substr(0, str.find_last_of("}"));
				//std::cout << sstr << std::endl;

				Jzon::Node clientJson = parser.parseString(sstr.substr(0, sstr.find_last_of("}") + 1));
				if (!clientJson.isValid()) {
					std::cout << "Error :" << parser.getError() << std::endl;
				}

				std::string action = clientJson.get("Action").toString();

				std::cout << action << std::endl;

				if (action == "move") {
					tMove_toClient.load(clientJson);

					// If exist, edit position.
					if (players.find(tMove_toClient.id) != players.end()) {
						players[tMove_toClient.id]->setPosition(tMove_toClient.x, tMove_toClient.y);
					}
					/*else {
						PlayerSprite *ps = new PlayerSprite(tMove_toClient.id);
						ps->setPosition(tMove_toClient.x, tMove_toClient.y);
						players.insert(std::pair<int, PlayerSprite*>(tMove_toClient.id, ps));
					}*/
				}
				else if (action == "fight") {
					std::cout << "LUCHA" << std::endl;
					globalVars.battle = !globalVars.battle;
					if (globalVars.battle) {
						//globalVars.fightState = FightStates::INIT;
						globalVars.id_fight = clientJson.get("Id_enemy").toInt();
						std::cout << "Luchando contra " << globalVars.id_fight << std::endl;

						// Escondo el resto de jugadores a excepción de mí y de mi enemigo
						PlayerSprite * aux;
						for (auto& kv : players) {
							aux = kv.second;
							if (globalVars.id_fight != kv.first) {
								aux->setVisibility(false);
							}
						}
					}
				}
				else if (action == "new") {
					tNew_toClient.load(clientJson);

					PlayerSprite *ps = new PlayerSprite(tNew_toClient.id);
					ps->setPosition(tNew_toClient.x, tNew_toClient.y);
					players.insert(std::pair<int, PlayerSprite*>(tNew_toClient.id, ps));
				}
				else if (action == "exit") {
					tExit_toClient.load(clientJson);
					players.erase(tExit_toClient.id);
				}
				else if (action == "finishBattle") {
					int die = clientJson.get("die").toBool();
					//TODO?!
				}
				else if (action == "hide") {
					Jzon::Node hides = tHide_toClient.load(clientJson);
					for (auto it = hides.begin(); it != hides.end(); it++) {
						std::cout << (*it).second.toInt() << std::endl;
						players[(*it).second.toInt()]->setVisibility(false);
					}
				}
				else if (action == "remObj") {
					tRemObj_toClient.load(clientJson);
					keyObjects[tRemObj_toClient.obj]->setDisponible(false);
					keyObjects[tRemObj_toClient.obj]->setVisible(false);
				}
				else if (action == "addObj") {
					tAddObj_toClient.load(clientJson);
					addKey(tAddObj_toClient.id_obj, tAddObj_toClient.color, tAddObj_toClient.posX, tAddObj_toClient.posY);
				}
				else if (action == "getObjFromServer") {
					tGetObjFromServer_toClient.load(clientJson);

					if (tGetObjFromServer_toClient.ok == 1) {
						//std::cout << "Colission " << kv.first << std::endl;
						collisionKey = tGetObjFromServer_toClient.id;
						std::cout << "Recogido objeto " << collisionKey << std::endl;
						objectFooterString = DEFAULT_OBJECTS_TEXT;
						std::stringstream sstm;
						sstm << DEFAULT_OBJECTS_TEXT;
						for (auto& kv : userObjects) {
							if (kv.second != NULL)
								sstm << kv.first << ", ";
						}
						sstm << collisionKey;
						objectFooterString = sstm.str();
						objectFooterText.setString(objectFooterString);
						std::cout << objectFooterString << std::endl;
						goP = false;
						globalVars.objsServerWait.erase(tGetObjFromServer_toClient.id);

						keyObjects[tGetObjFromServer_toClient.id]->setPosition(animatedSprite.getPosition());
						keyObjects[tGetObjFromServer_toClient.id]->setVisible(false);
						keyObjects[tGetObjFromServer_toClient.id]->setDisponible(true);
						userObjects.insert(std::pair<int, Key*>(tGetObjFromServer_toClient.id, keyObjects[tGetObjFromServer_toClient.id]));
					}
				}
				else if ("sendInitFight" == action) {
					tGetInitFight_toClient.load(clientJson);
					std::string msg = tSendAckInitFight_toServer.generate(tGetInitFight_toClient.id_battle);
					
					socket.send(msg.c_str(), msg.length());
				}
				else if ("okInitFight" == action) {
					tOkInitFight_toClient.load(clientJson);
					std::string msg = tOkInitFight_toServer.generate(tOkInitFight_toClient.id_battle);

					socket.send(msg.c_str(), msg.length());
					
				}
				
				memset(buffer_, '\0', SEMIBUFF);
			}
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
					window.close();
				if (event.key.code == sf::Keyboard::Y) {
					if (event.type == sf::Event::KeyPressed) {
						if (!pulsaY) {
							pulsaY = true;
							goY = !goY;
							std::cout << goY << std::endl;
						}
					}
					else if (event.type == sf::Event::KeyReleased) {
						pulsaY = false;
					}
				}
				if (event.key.code == sf::Keyboard::P) {
					if (event.type == sf::Event::KeyPressed) {
						if (!pulsaP) {
							pulsaP = true;
							goP = !goP;
							std::cout << goP << std::endl;
						}
					}
					else if (event.type == sf::Event::KeyReleased) {
						pulsaP = false;
					}
				}
			}
			globalVars.speed = goY ? 160.f : 80.f;

			sf::Time frameTime = frameClock.restart();

			// if a key was pressed set the correct animation and move correctly
			sf::Vector2f movement = movePlayer(animatedSprite);
			updateZoom(animatedSprite);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
				movement.x *= 4;
				movement.y *= 4;
			}

			if (userObjects[1] != NULL && sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
				releaseObject(1, animatedSprite, objectFooterString, objectFooterText, socket);
			}
			else if (userObjects[2] != NULL && sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
				releaseObject(2, animatedSprite, objectFooterString, objectFooterText, socket);
			}
			else if (userObjects[3] != NULL && sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
				releaseObject(3, animatedSprite, objectFooterString, objectFooterText, socket);
			}
			else if (userObjects[4] != NULL && sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
				releaseObject(4, animatedSprite, objectFooterString, objectFooterText, socket);
			}
			else if (goP && sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
				for (auto& kv : keyObjects) {
					if (globalVars.objsServerWait.find(kv.first) == globalVars.objsServerWait.end()) {
						Key *k = kv.second;
						if (k->isVisible() && k->getGlobalBounds().intersects(animatedSprite.getGlobalBounds())) {
							// TODO: Avisar al servidor quiere coger el objeto kv.first
							std::string message = tGetObj_toServer.generate(kv.second->getId(), globalVars.id);
							message += "\n";
							socket.send(message.c_str(), message.size());
							globalVars.objsServerWait.insert(kv.first);
						}
					}
				}
			}

			if (sf::Keyboard::isKeyPressed(keyboard[FIGHT])) {
				for (auto& kv : players) {
					PlayerSprite *ps = kv.second;
					if (ps->getVisibility() && ps->getGlobalBounds().intersects(animatedSprite.getGlobalBounds())) {
						std::cout << "Colisión enemiga" << std::endl;
						collisionEnemy = ps->getId();
						break;
					}
				}
			}

			animatedSprite.play(*currentAnimation);

			pos = movement * frameTime.asSeconds();

			animatedSprite.move(pos);
			globalVars.cam.move(pos);

			objectFooterBg.move(pos);
			objectFooterText.move(pos);
			objectHeaderBg.move(pos);
			objectHeaderText.move(pos);

			pos = animatedSprite.getPosition();
			//std::cout << finalTile.left << " " << finalTile.top << " -- " << animatedSprite.getGlobalBounds().left << " " << animatedSprite.getGlobalBounds().height << std::endl;
			/*if (finalTile.intersects(animatedSprite.getGlobalBounds())) {
				std::cout << "FIIIIIIIIIIN" << std::endl;
				break; // TODO: Nos piramos a un mensaje de alerta
			}*/
			window.setView(globalVars.cam);

			// if no key was pressed stop the animation
			if (globalVars.noKeyWasPressed) {
				animatedSprite.stop();
			}
			else {
				// Si hay movimento, aviso al servidor
				if (frameClock2.getElapsedTime().asSeconds() > sendtoServerTime) {
					std::string message = tMove_toServer.generate(animatedSprite.getGlobalBounds(), globalVars.id);
					message += "\n";
					socket.send(message.c_str(), message.size());
					frameClock2.restart();
				}
			}
			globalVars.noKeyWasPressed = true;

			// update AnimatedSprite
			animatedSprite.update(frameTime);
		}
		// draw
		window.clear();
		window.draw(globalVars.map);

		if (collisionKey >= 0) {
			keyObjects[collisionKey]->setVisible(false);
			userObjects[collisionKey] = keyObjects[collisionKey];
			collisionKey = -1;
		}
		else if (collisionEnemy >= 0) {
			std::string message = tInitFight_toServer.generate(collisionEnemy);
			socket.send(message.c_str(), message.size());
			globalVars.id_fight = collisionEnemy;
			std::cout << "Luchando contra " << globalVars.id_fight << std::endl;
			collisionEnemy = -1;
			globalVars.battle = true;
			globalVars.fightState = FightStates::WAITFORINIT;
			//globalVars.fightState = FightStates::INIT;
		}
		/*else if (animatedSprite.getGlobalBounds().contains(sf::Vector2f(finalTile.left, finalTile.top))) {  //).intersects(finalTile)) {
			std::cout << "Colisionando con fin del juego" << std::endl;
			finishGame = true;
		}*/
		std::for_each(keyObjects.begin(), keyObjects.end(), [&](std::pair<int, Key*> kv){
			if (kv.second->isVisible()) {
				window.draw(*kv.second);
			}
		});
		std::for_each(players.begin(), players.end(), [&](std::pair<int, PlayerSprite*> kv){
			if (kv.second->getVisibility()) {
				window.draw(*kv.second);
			}
		});

		window.draw(animatedSprite);

		window.draw(objectFooterBg);
		window.draw(objectFooterText);
		if (!userObjects.empty()) {
			window.draw(objectHeaderBg);
			window.draw(objectHeaderText);
		}

		if (globalVars.battle) {
			/*sf::Color color = sf::Color::Black;
			color.a = titleBattleInitOpacity;
			window.clear(color);
			switch (globalVars.fightState) {
				case FightStates::INIT: {
					if (frameClock.getElapsedTime().asSeconds() > 0.15) {
						std::cout << frameClock.getElapsedTime().asSeconds() << std::endl;
						titleBattleInitOpacity -= 15;
						fight_bg.setColor(sf::Color(255, 255, 255, titleBattleInitOpacity));
						frameClock.restart();
					}
					window.draw(fight_bg);

					if (titleBattleInitOpacity == 0) {
						globalVars.fightState = FightStates::DIE_RANDOM;

						dieRollLabel.setPosition(animatedSprite.getPosition());
						dieRollLabel.setScale(sf::Vector2f(5, 5));
						dieRollLabel.move(0, 20);
					}

					break;
				}
				case FightStates::DIE_RANDOM: {
					// TODO: Centrar número
					if (frameClock.getElapsedTime().asSeconds() > 0.15) {
						vDadoInt = (rand() % 6) + 1;
						dieRollLabel.setString(std::to_string(vDadoInt));
						frameClock.restart();
					}
					window.draw(dieRollLabel);
					break;
				}
				case FightStates::DIE_PLAYER: {
					std::cout << "Pedimos al server el valor final" << std::endl;
					std::string str = tDiePlayer_toServer.generate(globalVars.id_fight);
					socket.send(str.c_str(), str.size());
					globalVars.fightState = FightStates::FINISH;
					break;
				}
				case FightStates::FINISH: {
					break;
				}
				case FightStates::QUIT: {
					std::cout << "And the winner is ... " << std::endl;
					if (socket.receive(buffer_, SEMIBUFF, received) == sf::Socket::Done) {
						std::string sstr = buffer_;
						sstr = sstr.substr(0, str.find_last_of("}"));
						std::cout << sstr << std::endl;
						//fixBufferTCP(buffer, sstr);

						Jzon::Node clientJson = parser.parseString(sstr.substr(0, sstr.find_last_of("}") + 1));
						if (!clientJson.isValid()) {
							std::cout << "Error :" << parser.getError() << std::endl;
						}

						tFinishBattle_toClient.load(clientJson);

						if (tFinishBattle_toClient.winner == -1)
						{
							std::cout << "TIE :D:" << std::endl;
							globalVars.battle = false;
						}
						else if (tFinishBattle_toClient.winner == globalVars.id)
						{
							std::cout << "YOU WIN!!!" << std::endl;
							globalVars.battle = false;
						}
						else
						{
							std::cout << "YOU LOSE ..." << std::endl;
							finishGame = true;
						}
					}
					memset(buffer_, '\0', SEMIBUFF);
					globalVars.fightState = FightStates::NOT_BATTLE;
					titleBattleInitOpacity = MAX_OPACITY;
					break;
				}
			}*/
		}
		if (finishGame) {
			break;
		}
		window.display();
	}

	// Aviso al servidor de mi desconexión
	sendExitClient(socket);

	std::cout << "Vamos a fin" << std::endl;

	sf::Texture finishTexture;
	loadTexture(finishTexture, "finish.png");

	sf::Sprite finishSprite(finishTexture);

	finishSprite.setPosition(0, 0);

	window.setView(globalVars.origCam);

	while (window.isOpen()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
			window.close();
		}
		window.clear();
		window.draw(finishSprite);
		window.display();
	}
	std::cout << "FIN" << std::endl;
	return 0;
}

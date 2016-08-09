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

#pragma once

#include <string>
#include "Jzon.h"
#include <SFML/Graphics.hpp>
#include "Key.h"

namespace TCP {
	namespace ClientToServer {
		struct TInitWithName {
			std::string generate(std::string name) {
				Jzon::Node root(Jzon::Node::T_OBJECT);
				root.add("Action", "initWName");
				root.add("Name", name);
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TGetObject {
			std::string generate(int id_obj, int& id_user) {
				Jzon::Node root(Jzon::Node::T_OBJECT);
				root.add("Action", Jzon::Node(Jzon::Node::T_STRING, "getObj"));
				root.add("Id_obj", id_obj);
				root.add("Id_user", id_user);
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TFreeObject {
			std::string generate(Key& key, int& id_user) {
				Jzon::Node root(Jzon::Node::T_OBJECT);
				root.add("Action", Jzon::Node(Jzon::Node::T_STRING, "freeObj"));
				Jzon::Node obj(Jzon::Node::T_OBJECT);
				obj.add("Id_obj", Jzon::Node(key.getId()));
				obj.add("PosX", Jzon::Node(key.getPosition().x));
				obj.add("PosY", Jzon::Node(key.getPosition().y));
				root.add("Obj", obj);
				root.add("Id_user", Jzon::Node(id_user));
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TMove {
			std::string generate(sf::FloatRect& fr, int& id) {
				Jzon::Node root(Jzon::Node::T_OBJECT);
				root.add("Action", Jzon::Node(Jzon::Node::T_STRING, "move"));
				Jzon::Node position(Jzon::Node::T_OBJECT);
				position.add("X", Jzon::Node(fr.left));
				position.add("Y", Jzon::Node(fr.top));
				root.add("Pos", position);
				root.add("Id", Jzon::Node(id));
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TExit {
			std::string generate() {
				Jzon::Node root(Jzon::Node::T_OBJECT);
				root.add("Action", Jzon::Node("exit"));
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TDiePlayer {
			std::string generate(int& id_fight) {
				Jzon::Node needDie(Jzon::Node::T_OBJECT);
				needDie.add("Action", "finishBattle");
				needDie.add("Id_enemy", id_fight);
				Jzon::Writer w;
				std::string str = "";
				w.writeString(needDie, str);
				return str;
			}
		};
		struct TOkInitFight {
			std::string generate(int& id_battle) {
				Jzon::Node root = Jzon::object();
				root.add("Action", "okInitFight");
				root.add("Id_battle", id_battle);
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TSendAckInitFight {
			std::string generate(int& id_battle) {
				Jzon::Node root = Jzon::object();
				root.add("Action", "sendAckInitFight");
				root.add("Id_battle", id_battle);
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
		struct TInitBattle {
			std::string generate(int& id_enemy) {
				Jzon::Node root = Jzon::object();
				root.add("Action", "initFight");
				root.add("Id_enemy", id_enemy);
				Jzon::Writer w;
				std::string str = "";
				w.writeString(root, str);
				return str;
			}
		};
	}

	namespace ServerToClient {
		struct TMove {
			float x;
			float y;
			int id;
			void load(Jzon::Node& node) {
				Jzon::Node pos = node.get("Pos");
				this->x = pos.get("X").toFloat();
				this->y = pos.get("Y").toFloat();
				this->id = node.get("Id").toInt();
			}
		};
		/*struct TFight {

		};*/
		struct TNew {
			float x;
			float y;
			int id;
			void load(Jzon::Node& node) {
				this->x = node.get("PosX").toFloat();
				this->y = node.get("PosY").toFloat();
				this->id = node.get("Id").toInt();
			}
		};
		struct TExit {
			float id;
			void load(Jzon::Node& node) {
				this->id = node.get("Id").toInt();
			}
		};
		struct THide {
			Jzon::Node& load(Jzon::Node& node) {
				return node.get("Ids");
			}
		};
		struct TFinishBattle {
			int winner;
			int valueMe;
			int valueEnemy;
			void load(Jzon::Node& node) {
				this->winner = node.get("Winner").toInt();
				this->valueMe = node.get("ValueClient").toInt();
				this->valueEnemy = node.get("ValueEnemy").toInt();
			}
		};
		struct TRemObject {
			int obj;
			void load(Jzon::Node& node) {
				this->obj = node.get("Id_obj").toInt();
			}
		};
		struct TAddObj {
			int id_obj;
			float posX;
			float posY;
			std::string color;
			void load(Jzon::Node& node) {
				node = node.get("Obj");
				this->id_obj = node.get("Id").toInt();
				this->posX = node.get("PosX").toFloat();
				this->posY = node.get("PosY").toFloat();
				this->color = node.get("Color").toString();
			}
		};
		struct TGetObjFromServer {
			int id;
			int ok;
			void load(Jzon::Node& node) {
				this->id = node.get("Id").toInt();
				this->ok = node.get("OK").toInt();
			}
		};
		struct TLiberateObj {
			int id;
			int ok;
			void load(Jzon::Node& node) {
				this->id = node.get("Id").toInt();
				this->ok = node.get("OK").toInt();
			}
		};
		struct TGetInitFight {
			int id_battle;
			void load(Jzon::Node& node) {
				this->id_battle = node.get("Id_battle").toInt();
			}
		};
		struct TOkInitFight {
			int id_battle;
			void load(Jzon::Node& node) {
				this->id_battle = node.get("Id_battle").toInt();
			}
		};
	}
}
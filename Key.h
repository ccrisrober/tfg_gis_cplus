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

#include <SFML/Graphics.hpp>

class Key : public sf::Drawable, public sf::Transformable
{
public:
	typedef enum { Red, Blue, Yellow, Green } TKey;
	Key(TKey key, int id, sf::Vector2f position = sf::Vector2f(0, 0));
	static void initialize(/*std::string file*/);
	~Key();

	static std::string enumToString(TKey k) {
		std::string ret = "";
		switch (k)
		{
		case TKey::Red:
			ret = "Red";
			break;
		case TKey::Blue:
			ret = "Blue";
			break;
		case TKey::Yellow:
			ret = "Yellow";
			break;
		case TKey::Green:
			ret = "Green";
			break;
		}
		return ret;
	}
	static TKey intToEnum(int k) {
		TKey ret;
		switch (k)
		{
		case 1:
			ret = TKey::Red;
			break;
		case 2:
			ret = TKey::Blue;
			break;
		case 3:
			ret = TKey::Yellow;
			break;
		case 4:
			ret = TKey::Green;
			break;
		}
		return ret;
	}

	static int enumToInt(TKey k) {
		int ret = -1;
		switch (k)
		{
		case TKey::Red:
			ret = 1;
			break;
		case TKey::Blue:
			ret = 2;
			break;
		case TKey::Yellow:
			ret = 3;
			break;
		case TKey::Green:
			ret = 4;
			break;
		}
		return ret;
	}

	sf::FloatRect getLocalBounds() const;

	void setVisible(bool);
	bool isVisible();
	bool isDisponible() { return m_disponibility; }
	void setDisponible(bool v) { m_disponibility = v; if (!v) { m_visible = false; } }
	int getId() { return id; }
	sf::FloatRect getGlobalBounds() const;

	//void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
protected:
	static std::map<TKey, sf::IntRect> m_positions;
	static sf::Texture& m_texture;
	sf::Sprite m_sprite;
	TKey key;
	int id;
	bool m_visible;
	bool m_disponibility;
};


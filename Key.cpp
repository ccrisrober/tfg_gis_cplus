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

#include "Key.h"
#include <iostream>

std::map<Key::TKey, sf::IntRect> Key::m_positions;
sf::Texture& Key::m_texture = sf::Texture();

Key::Key(TKey key, int id, sf::Vector2f position) : key(key), m_visible(true)
{
	this->m_sprite = sf::Sprite(Key::m_texture, Key::m_positions[key]);
	this->setPosition(position);
	this->id = id;	//enumToString(key);
	this->m_disponibility = true;
}


Key::~Key()
{
}


void Key::setVisible(bool v) { m_visible = v; }
bool Key::isVisible() { return m_visible; }
sf::FloatRect Key::getLocalBounds() const
{
	//sf::IntRect rect = this->m_sprite.get

	//float width = static_cast<float>(std::abs(rect.width));
	//float height = static_cast<float>(std::abs(rect.height));

	return sf::FloatRect(0.f, 0.f, 64, 64); // width, height);
}

sf::FloatRect Key::getGlobalBounds() const
{
	return getTransform().transformRect(getLocalBounds());
}

void Key::initialize(/*std::string file*/) {
	if (!Key::m_texture.loadFromFile("objects.png"))
	{
		std::cout << "Failed to load player spritesheet!" << std::endl;
		throw;
	}
	Key::m_positions[Key::Blue] = sf::IntRect(0, 0, 64, 64);
	Key::m_positions[Key::Red] = sf::IntRect(64, 64, 64, 64);
	Key::m_positions[Key::Yellow] = sf::IntRect(0, 64, 64, 64);
	Key::m_positions[Key::Green] = sf::IntRect(64, 0, 64, 64);
}

/*void Key::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	//states.transform *= getTransform();
	target.draw(*m_sprite);
}*/

void Key::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(m_sprite, states);
}
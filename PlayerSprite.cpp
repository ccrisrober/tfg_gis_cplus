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

#include "PlayerSprite.h"


void PlayerSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(m_shape, states);
}

PlayerSprite::PlayerSprite(int id) : m_id(id), m_visibility(true) {
	m_shape.setSize(sf::Vector2f(64, 64));

	sf::Color color;

	int c = id % 5;
	switch (c)
	{
	case 0:
		color = sf::Color::Red;
		break;
	case 1:
		color = sf::Color::Blue;
		break;
	case 2:
		color = sf::Color::Green;
		break;
	case 3:
		color = sf::Color::Yellow;
		break;
	case 4:
		color = sf::Color::Magenta;
		break;
	default:
		color = sf::Color::Black;
		break;
	}

	m_shape.setFillColor(color);

}

const int PlayerSprite::getId() const { return m_id; }

const bool PlayerSprite::getVisibility() const { return m_visibility; }


sf::FloatRect PlayerSprite::getLocalBounds() const
{
	//sf::IntRect rect = this->m_sprite.get

	//float width = static_cast<float>(std::abs(rect.width));
	//float height = static_cast<float>(std::abs(rect.height));

	return sf::FloatRect(0.f, 0.f, 64, 64); // width, height);
}

sf::FloatRect PlayerSprite::getGlobalBounds() const
{
	return getTransform().transformRect(getLocalBounds());
}

void PlayerSprite::setOpacity(int alpha) {
	sf::Color c = this->m_shape.getFillColor();
	c.a = alpha;
	this->m_shape.setFillColor(c);
}
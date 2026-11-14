#include "ColorBuilder.h"
#include "VectorBuilder.h"

#include <cstdint>
#include <format>
#include <glm/vec3.hpp>

const WallpaperEngine::Data::Model::Color WallpaperEngine::Data::Builders::ColorBuilder::White
    = WallpaperEngine::Data::Model::Color (1.0f, 1.0f, 1.0f, 1.0f);
const WallpaperEngine::Data::Model::Color WallpaperEngine::Data::Builders::ColorBuilder::Black
    = WallpaperEngine::Data::Model::Color (0.0f, 0.0f, 0.0f, 1.0f);

WallpaperEngine::Data::Model::Color
WallpaperEngine::Data::Builders::ColorBuilder::parse (const std::string& value, float alpha) {
    // hex colors should be converted to int colors
    if (value.find ('#') == 0) {
	auto number = value.substr (1);

	// expand short css notation into the right one
	// support for css notation
	if (number.size () == 3) {
	    number = std::format (
		"{}{}{}{}{}{}{:02x}", number.at (0), number.at (0), number.at (1), number.at (1), number.at (2),
		number.at (2), static_cast<int> (alpha * 255)
	    );
	} else if (number.size () == 4) {
	    number = std::format (
		"{}{}{}{}{}{}{}{}", number.at (0), number.at (0), number.at (1), number.at (1), number.at (2),
		number.at (2), number.at (3), number.at (3)
	    );
	} else if (number.size () == 6) {
	    // RRGGBB has no alpha byte, add it like the short form does
	    number += std::format ("{:02x}", static_cast<int> (alpha * 255));
	} else if (number.size () != 8) {
	    sLog.exception ("Invalid CSS color notation for ", value);
	}

	// parse hex color; RRGGBBAA needs all 32 bits, which overflows a signed int
	// for any red above 7f
	std::size_t parsed = 0;
	const auto color = static_cast<uint32_t> (std::stoul (number, &parsed, 16));

	if (parsed != number.size ()) {
	    sLog.exception ("Invalid CSS color notation for ", value);
	}

	return WallpaperEngine::Data::Model::Color (
	    (color >> 24 & 0xFF) / 255.0f, (color >> 16 & 0xFF) / 255.0f, (color >> 8 & 0xFF) / 255.0f,
	    (color & 0xFF) / 255.0f
	);
    }

    int vectorSize = VectorBuilder::preparseSize (value);

    if (vectorSize != 3 && vectorSize != 4) {
	throw std::invalid_argument ("Invalid color value");
    }

    // Retain the normalized-integer convention; only consumed decimal components affect it.
    if (!VectorBuilder::hasDecimalPoint (value, vectorSize)) {
	const auto final = vectorSize == 3 ? glm::ivec4 (VectorBuilder::parse<glm::ivec3> (value), alpha * 255)
					   : VectorBuilder::parse<glm::ivec4> (value);

	return { final.r / 255.0f, final.g / 255.0f, final.b / 255.0f, final.a / 255.0f };
    }

    return Model::Color (
	vectorSize == 3 ? glm::vec4 (VectorBuilder::parse<glm::vec3> (value), alpha)
			: VectorBuilder::parse<glm::vec4> (value)
    );
}
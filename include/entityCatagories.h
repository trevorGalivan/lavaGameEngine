#pragma once

namespace catagories {
	// bitmask used to catagorize entities.
	// objects may belong to one, more than one, or no catagories at all
	enum catagories : unsigned int {
		worldType = 1u, // immoveable objects like platforms and walls
		playerType = 1u << 1u, // player character (NOT player projectiles, props, etc)
		projectileType = 1u << 2u,
		propsType = 1u << 3u,
		miscType = 1u << 4u,


		// The top 8 bits are reserved for team affiliation
		team1Type = 1u << 24u,
		team2Type = 1u << 25u,
		team3Type = 1u << 26u,
		team4Type = 1u << 27u,
		team5Type = 1u << 28u,
		team6Type = 1u << 29u,
		team7Type = 1u << 30u,
		team8Type = 1u << 31u,
	};

	// takes a 1-based player number (E.G. player 1, player 2...) and returns the corresponding bit for that team
	// player nums above the appropriate range (>8) will result in no bits being set
	inline unsigned int teamBit(unsigned int playerNum) {
		return team1Type << (playerNum - 1);
	}

	// gets only the bits corresponding to the team
	inline unsigned int getTeamBits(unsigned int catagory) {
		return catagory & 0xFF000000u;
	}
	// gets only the bits corresponding to the type
	inline unsigned int clearTeamBits(unsigned int catagory) {
		return catagory & 0x00FFFFFFu;
	}

	// returns true if both catagories share ANY common types
	inline bool sameTeam(unsigned int catagory1, unsigned int catagory2) {
		return getTeamBits(catagory1 & catagory2);
	}

	// returns true if both catagories share ANY common teams
	inline bool sameType(unsigned int catagory1, unsigned int catagory2) {
		return clearTeamBits(catagory1 & catagory2);
	}
}

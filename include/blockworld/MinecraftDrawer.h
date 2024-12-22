#ifndef MINECRAFTDRAWER_H
#define MINECRAFTDRAWER_H

#include <protogen/ICanvas.hpp>
#include <blockworld/minecraft_state.h>

namespace blockworld {

using protogen::ICanvas;

class MinecraftDrawer final {
public:
	MinecraftDrawer();
	void draw(ICanvas& canvas, const MinecraftState& state) const;
private:
	static void drawWorld(ICanvas& canvas, const BlockMatrix& block_matrix, const BlockColorProfile& color_profile);
	static void drawPlayers(ICanvas& canvas, const MinecraftState& state);
	static void drawPlayer(ICanvas& canvas, const MinecraftPlayerState& player_state, const BlockColorProfile& color_profile);
};

}

#endif
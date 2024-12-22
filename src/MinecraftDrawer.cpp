#include <blockworld/MinecraftDrawer.h>

namespace blockworld {

MinecraftDrawer::MinecraftDrawer()
{}

void MinecraftDrawer::draw(ICanvas& canvas, const MinecraftState& state) const {
    drawWorld(canvas, state.blockMatrix(), state.blockColorProfile());
    drawPlayers(canvas, state);
}

void MinecraftDrawer::drawWorld(ICanvas& canvas, const BlockMatrix& block_matrix, const BlockColorProfile& color_profile) {
    for(std::size_t r = 0; r < block_matrix.rows(); r++)
    {
        for(std::size_t c = 0; c < block_matrix.cols(); c++)
        {
            const auto color = color_profile(block_matrix.get(r, c).value());
            canvas.setPixel(c, r, std::get<0>(color), std::get<1>(color), std::get<2>(color));
        }
    }
}

void MinecraftDrawer::drawPlayers(ICanvas& canvas, const MinecraftState& state) {
    const auto players = state.players();
    for(const auto& player_id : players) {
        state.accessPlayer(player_id, [&canvas, &state](const MinecraftPlayerState& player_state){
            drawPlayer(canvas, player_state, state.blockColorProfile());
        });
    }
}

void MinecraftDrawer::drawPlayer(ICanvas& canvas, const MinecraftPlayerState& player_state, const BlockColorProfile& color_profile) {
    const auto color = color_profile(player_state.selectedBlock());
    const auto cursor = player_state.cursor();
    canvas.setPixel(cursor.second, cursor.first, std::get<0>(color), std::get<1>(color), std::get<2>(color));
}

} // namespace
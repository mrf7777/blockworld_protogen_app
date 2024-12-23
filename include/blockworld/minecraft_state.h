#ifndef MINECRAFT_STATE_H
#define MINECRAFT_STATE_H

#include <utility>
#include <string>
#include <mutex>
#include <map>

#include <blockworld/minecraft.h>

namespace blockworld {

class MinecraftPlayerState final {
public:

    using CursorPos = std::pair<std::size_t, std::size_t>;

	enum class CursorDirection {
		Up, Down, Left, Right,
	};
	static CursorDirection stringToCursorDirection(const std::string& cd);
	
	MinecraftPlayerState(std::size_t start_row, std::size_t start_col, std::size_t max_row, std::size_t max_col);

	Block selectedBlock() const; 
	void setSelectedBlock(const Block& b);
	CursorPos cursor() const;
	void moveCursor(CursorDirection direction);
private:
	void correctCursor();

	CursorPos m_cursor;
	std::size_t m_maxRow;
	std::size_t m_maxCol;
	Block m_selectedBlock;
};

class MinecraftState final {
public:
	using PlayerId = std::string;

	MinecraftState();
	const BlockMatrix& blockMatrix() const;
	BlockMatrix& blockMatrix();
	const BlockColorProfile blockColorProfile() const;
	bool addNewPlayer(const PlayerId& id);
	void removePlayer(const PlayerId& id);
	bool accessPlayer(const PlayerId& id, std::function<void(MinecraftPlayerState&)> accessor);
	bool accessPlayer(const PlayerId& id, std::function<void(const MinecraftPlayerState&)> accessor) const;
	std::vector<PlayerId> players() const;
	std::string playersSeparatedByNewline() const;
private:
	std::vector<PlayerId> _players() const;

	BlockMatrix m_blockMatrix;
	BlockColorProfile m_blockColorProfile;
	mutable std::mutex m_playerMutex;
	std::map<PlayerId, MinecraftPlayerState> m_players;
};

}	// namespace

#endif
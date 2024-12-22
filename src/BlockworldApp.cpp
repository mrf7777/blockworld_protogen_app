#include <protogen/IProtogenApp.hpp>
#include <protogen/IProportionProvider.hpp>
#include <protogen/Resolution.hpp>
#include <blockworld/minecraft.h>
#include <blockworld/MinecraftDrawer.h>
#include <blockworld/minecraft_state.h>
#include <cmake_vars.h>

#include <cmath>

using namespace protogen;
using namespace blockworld;

class BlockworldApp : public IProtogenApp {
public:
    BlockworldApp()
        : IProtogenApp(),
        m_active(false),
        m_state{},
        m_deviceResolution{0, 0}
    {}

    std::string name() const override {
        return "Blockworld";
    }

    std::string id() const override {
        return PROTOGEN_APP_ID;
    }

    std::string description() const override {
        return "An imitation of multiplayer Minecraft in 2D on a Protogen display.";
    }

    bool sanityCheck([[maybe_unused]] std::string& errorMessage) const override {
        return true;
    }

    void setActive(bool active) override {
        m_active = active;
    }

    Endpoints serverEndpoints() const override {
        using httplib::Request, httplib::Response;
        return Endpoints{
            {
                Endpoint{HttpMethod::Put, "/world/generate"},
                [this](const Request& req, Response&) {
                    const std::size_t seed = std::hash<std::string>{}(req.body);
                    const auto world = BlockMatrixGenerator(m_deviceResolution.height(), m_deviceResolution.width()).generate(seed);
                    m_state.blockMatrix() = world;
                }
            },
            {
                Endpoint{HttpMethod::Get, "/players"},
                [this](const Request&, Response& res){
                    res.set_content(m_state.playersSeparatedByNewline(), "text/plain");
                }
            },
            {
                Endpoint{HttpMethod::Put, "/players/:player"},
                [this](const Request& req, Response&){
                    m_state.addNewPlayer(req.path_params.at("player"));
                }
            },
            {
                Endpoint{HttpMethod::Delete, "/players/:player"},
                [this](const Request& req, Response&){
                    m_state.removePlayer(req.path_params.at("player"));
                }
            },
            {
                Endpoint{HttpMethod::Post, "/players/:player/move"},
                [this](const Request& req, Response&){
                    const auto player_id = req.path_params.at("player");
                    const auto move_direction = MinecraftPlayerState::stringToCursorDirection(req.body);
                    m_state.accessPlayer(player_id, [move_direction](MinecraftPlayerState& player_state){
                        player_state.moveCursor(move_direction);
                    });
                }
            },
            {
                Endpoint{HttpMethod::Post, "/players/:player/place_block"},
                [this](const Request& req, Response&){
                    const auto player_id = req.path_params.at("player");
                    MinecraftPlayerState::CursorPos player_cursor;
                    Block player_block;
                    m_state.accessPlayer(player_id, [&player_cursor, &player_block](MinecraftPlayerState& player_state){
                        player_cursor = player_state.cursor();
                        player_block = player_state.selectedBlock();
                    });
                    m_state.blockMatrix().set(
                        player_cursor.first,
                        player_cursor.second,
                        player_block
                    );
                }
            },
            {
                Endpoint{HttpMethod::Put, "/players/:player/block"},
                [this](const Request& req, Response&){
                    const auto player_id = req.path_params.at("player");
                    const auto block = Block::fromString(req.body);
                    m_state.accessPlayer(player_id, [block](MinecraftPlayerState& player_state){
                        player_state.setSelectedBlock(block);
                    });
                }
            },
            {
                Endpoint{HttpMethod::Get, "/blocks"},
                [this](const Request&, Response& res){
                    res.set_content(Block::allBlocksSeparatedByNewline(), "text/plain");
                }
            },
            {
                Endpoint{HttpMethod::Get, "/blocks/:block/color"},
                [this](const Request& req, Response& res){
                    const auto block = Block::fromString(req.path_params.at("block"));
                    const auto block_color = m_state.blockColorProfile()(block);
                    const auto color_hex = colorHexCodeFromColor(block_color);
                    res.set_content(color_hex, "text/plain");
                }
            },
        };
    }

    std::string homePage() const override {
        return "/static/minecraft.html";
    }

    std::string staticFilesDirectory() const override {
        return "/static";
    }

    std::string staticFilesPath() const override {
        return "/static";
    }

    std::string thumbnail() const override {
        return "/static/thumbnail.png";
    }

    void render(ICanvas& canvas) const override {
        m_drawer.draw(canvas, m_state);
    }

    float framerate() const override {
        return 15;
    }

    std::vector<Resolution> supportedResolutions(const Resolution& device_resolution) const override {
        m_deviceResolution = device_resolution;
        return {device_resolution};
    }

    void setMouthProportionProvider([[maybe_unused]] std::shared_ptr<IProportionProvider> provider) {
    }

private:
    bool m_active;
    mutable MinecraftState m_state;
    mutable Resolution m_deviceResolution;
    MinecraftDrawer m_drawer;
};

// Interface to create and destroy you app.
// This is how your app is created and consumed as a library.
extern "C" IProtogenApp * create_app() {
    return new BlockworldApp();
}

extern "C" void destroy_app(IProtogenApp * app) {
    delete app;
}
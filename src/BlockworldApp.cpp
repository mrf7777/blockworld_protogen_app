#include <protogen/IProtogenApp.hpp>
#include <protogen/IProportionProvider.hpp>
#include <protogen/Resolution.hpp>
#include <protogen/StandardAttributeStore.hpp>
#include <blockworld/minecraft.h>
#include <blockworld/MinecraftDrawer.h>
#include <blockworld/minecraft_state.h>
#include <cmake_vars.h>

#include <cmath>
#include <memory>
#include <thread>

using namespace protogen;
using namespace blockworld;

class BlockworldApp : public IProtogenApp {
public:
    BlockworldApp()
        : IProtogenApp(),
        m_active(false),
        m_state{},
        m_deviceResolution{0, 0},
        m_attributes(std::shared_ptr<StandardAttributeStore>(new StandardAttributeStore())),
        m_webServerThread(),
        m_webServerPort(-1),
        m_resources()
    {
        using namespace protogen::attributes;
        using Access = protogen::attributes::IAttributeStore::Access;
        m_attributes->adminSetAttribute(ATTRIBUTE_ID, PROTOGEN_APP_ID, Access::Read);
        m_attributes->adminSetAttribute(ATTRIBUTE_NAME, "Blockworld", Access::Read);
        m_attributes->adminSetAttribute(ATTRIBUTE_DESCRIPTION, "An imitation of multiplayer Minecraft in 2D on a Protogen display.", Access::Read);
        m_attributes->adminSetAttribute(ATTRIBUTE_THUMBNAIL, "/static/thumbnail.png", Access::Read);
        m_attributes->adminSetAttribute(ATTRIBUTE_MAIN_PAGE, "/static/minecraft.html", Access::Read);
        m_attributes->adminSetAttribute(ATTRIBUTE_HOME_PAGE, "https://github.com/mrf7777/blockworld_protogen_app", Access::Read);
    }

    bool sanityCheck([[maybe_unused]] std::string& errorMessage) const override {
        return true;
    }

    void initialize() override {
        m_webServerThread = std::thread([this](){
            using httplib::Request, httplib::Response;
            auto server = httplib::Server();

            server.set_mount_point("/static", m_resources + "/static");

            server.Put("/world/generate", [this](const Request& req, Response&){
                const std::size_t seed = std::hash<std::string>{}(req.body);
                const auto world = BlockMatrixGenerator(m_deviceResolution.height(), m_deviceResolution.width()).generate(seed);
                m_state.blockMatrix() = world;
            });
            server.Get("/players", [this](const Request&, Response& res){
                res.set_content(m_state.playersSeparatedByNewline(), "text/plain");
            });
            server.Put("/players/:player", [this](const Request& req, Response&){
                m_state.addNewPlayer(req.path_params.at("player"));
            });
            server.Delete("/players/:player", [this](const Request& req, Response&){
                m_state.removePlayer(req.path_params.at("player"));
            });
            server.Post("/players/:player/move", [this](const Request& req, Response&){
                const auto player_id = req.path_params.at("player");
                const auto move_direction = MinecraftPlayerState::stringToCursorDirection(req.body);
                m_state.accessPlayer(player_id, [move_direction](MinecraftPlayerState& player_state){
                    player_state.moveCursor(move_direction);
                });
            });
            server.Post("/players/:player/place_block", [this](const Request& req, Response&){
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
            });
            server.Put("/players/:player/block", [this](const Request& req, Response&){
                const auto player_id = req.path_params.at("player");
                const auto block = Block::fromString(req.body);
                m_state.accessPlayer(player_id, [block](MinecraftPlayerState& player_state){
                    player_state.setSelectedBlock(block);
                });
            });
            server.Get("/blocks", [this](const Request&, Response& res){
                res.set_content(Block::allBlocksSeparatedByNewline(), "text/plain");
            });
            server.Get("/blocks/:block/color", [this](const Request& req, Response& res){
                const auto block = Block::fromString(req.path_params.at("block"));
                const auto block_color = m_state.blockColorProfile()(block);
                const auto color_hex = colorHexCodeFromColor(block_color);
                res.set_content(color_hex, "text/plain");
            });

            m_webServerPort = server.bind_to_any_port("0.0.0.0");
            server.listen_after_bind();
        });
        m_webServerThread.detach();
    }

    int webPort() const override {
        return m_webServerPort;
    }

    void setActive(bool active) override {
        m_active = active;
    }

    void receiveResourcesDirectory([[maybe_unused]] const std::string& resourcesDirectory) override {
        m_resources = resourcesDirectory;
    }

    void receiveUserDataDirectory([[maybe_unused]] const std::string& userDataDirectory) override {
    }

    void render(ICanvas& canvas) const override {
        m_drawer.draw(canvas, m_state);
    }

    float framerate() const override {
        return 15;
    }

    void receiveDeviceResolution(const Resolution& device_resolution) override {
        m_deviceResolution = device_resolution;
    }

    std::vector<Resolution> supportedResolutions() const override {
        return {m_deviceResolution};
    }

    void setMouthProportionProvider([[maybe_unused]] std::shared_ptr<IProportionProvider> provider) {
    }

    std::shared_ptr<attributes::IAttributeStore> getAttributeStore() override {
        return m_attributes;
    }

private:
    bool m_active;
    mutable MinecraftState m_state;
    Resolution m_deviceResolution;
    MinecraftDrawer m_drawer;
    std::shared_ptr<StandardAttributeStore> m_attributes;
    std::thread m_webServerThread;
    int m_webServerPort;
    std::string m_resources;
};

// Interface to create and destroy you app.
// This is how your app is created and consumed as a library.
extern "C" IProtogenApp * create_app() {
    return new BlockworldApp();
}

extern "C" void destroy_app(IProtogenApp * app) {
    delete app;
}
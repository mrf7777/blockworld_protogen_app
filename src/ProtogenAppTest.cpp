#include <protogen/IProtogenApp.hpp>
#include <protogen/IProportionProvider.hpp>
#include <protogen/Resolution.hpp>
#include <cmake_vars.h>

#include <cmath>

using namespace protogen;

class ProtogenAppTest : public protogen::IProtogenApp {
public:
    std::string name() const override {
        return "Protogen App Test";
    }

    std::string id() const override {
        return PROTOGEN_APP_ID;
    }

    std::string description() const override {
        return "This is a demo protogen app that is a simple template for education.";
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
                Endpoint{HttpMethod::Get, "/home"},
                [](const Request&, Response& res){ res.set_content("This is the homepage.", "text/html"); }
            },
            {
                Endpoint{HttpMethod::Get, "/hello"},
                [](const Request&, Response& res){ res.set_content("Hello!", "text/plain"); }
            },
            {
                Endpoint{HttpMethod::Get, "/hello/website"},
                [](const Request&, Response& res){ res.set_content("Hello, website!", "text/plain"); }
            },
        };
    }

    std::string homePage() const override {
        return "/static/index.html";
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
        if(m_mouthProvider) {
            const double mouth_open_proportion = m_mouthProvider->proportion();
            canvas.fill(0, std::floor(std::lerp(0.0, 255.0, mouth_open_proportion)), 0);
        } else {
            canvas.fill(127, 0, 0);
        }
    }

    float framerate() const override {
        return 10;
    }

    std::vector<Resolution> supportedResolutions(const Resolution& device_resolution) const override {
        return {device_resolution};
    }

    void setMouthProportionProvider(std::shared_ptr<IProportionProvider> provider) {
        m_mouthProvider = provider;
    }

private:
    std::shared_ptr<IProportionProvider> m_mouthProvider;
    bool m_active;
};

// Interface to create and destroy you app.
// This is how your app is created and consumed as a library.
extern "C" IProtogenApp * create_app() {
    return new ProtogenAppTest();
}

extern "C" void destroy_app(IProtogenApp * app) {
    delete app;
}
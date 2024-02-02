#pragma once

#include "Layer/Layer.h"

namespace mccinfo {
struct ApplicationSpecification {
    std::string Name = "mccinfo-monitor";
    uint32_t Width = 450;
    uint32_t Height = 200;
};

class Application {
  public:
    Application(
        const ApplicationSpecification &applicationSpecification = ApplicationSpecification());
    ~Application();

    void Run();
    void Close();

    static Application *Instance();
    uint32_t &GetWidth() {
        return m_Specification.Width;
    }
    uint32_t &GetHeight() {
        return m_Specification.Height;
    }

    template <typename T> void PushLayer() {
        static_assert(std::is_base_of<mccinfo::Layer, T>::value,
                      "Pushed type is not subclass of Layer!");
        m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
    }

    void PushLayer(const std::shared_ptr<mccinfo::Layer> &layer) {
        m_LayerStack.emplace_back(layer);
        layer->OnAttach();
    }

  private:
    void Init();
    void Shutdown();

    void StartNewAppFrame();
    void RenderAppFrame();

    void FixTimestep();

  private:
    void CenterWindow();
    void ApplyBorderlessFrame();
    void ApplyBgColors();


  private:
    HWND m_hHWND;
    std::unique_ptr<BorderlessWindow> m_Window;
    WNDCLASSEXW m_hWC;
    ApplicationSpecification m_Specification;
    std::vector<std::shared_ptr<mccinfo::Layer>> m_LayerStack;
    bool m_Running = false;

    HANDLE m_hFrameIRTimer;
};
} // namespace mccinfo
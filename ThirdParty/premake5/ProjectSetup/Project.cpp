#include "Haketon/Application.h"
#include "Haketon/EntryPoint.h"

class ExampleLayer : public Haketon::Layer
{
public:
    virtual void OnUIRender() override
    {
        ImGui::Begin("Hello");
        ImGui::Button("Button");
        ImGui::End();

        ImGui::ShowDemoWindow();
    }
};

Haketon::Application* Haketon::CreateApplication(int argc, char** argv)
{
    Haketon::ApplicationSpecification spec;
    spec.Name = "Haketon Project";

    Haketon::Application* app = new Haketon::Application(spec);
    app->PushLayer<ExampleLayer>();
    app->SetMenubarCallback([app]()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                app->Close();
            }

            ImGui::EndMenu();
        }
    });

    return app;
}

#include "pch.h"
#include "Monitor.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
extern HGLRC g_hRC;

//std::shared_ptr<mcctp::Image> LoadAppIcon(void);
//void DoTexturePackCheckbox(mcctp::TexturePackFlags texture_pack, bool *active);
//void DoAllTexturePackCheckboxes(void);
namespace mccinfo {

std::pair<ImVec2, ImVec2> GetScreenFill(void);
std::pair<ImVec2, ImVec2> GetScreenMaintainAspectRatio(uint32_t width, uint32_t height);
std::pair<ImVec2, ImVec2> GetItemRectMaintainAspectRatio(uint32_t width, uint32_t height, ImVec2 item_extents);
//void SortResources(std::vector<mcctp::TexturePackResource> &resources);


Monitor::Monitor() {
    /*
  //m_TexturePacksPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  m_TexturePacksPath = std::filesystem::path(std::filesystem::current_path()).parent_path().parent_path().parent_path();
  m_TexturePacksPath += "/ui/texturepacks";

  m_DumpFormatFlag = mcctp::DumpFormatFlags::Native;
  m_CompressionFlag = mcctp::DumpCompressionFlags::None;

  mcctp::Initialize(m_TexturePacksPath, mcctp::TexturePackFlags::All);

  std::filesystem::path app_icon =
      std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  app_icon += "\\res\\mcctp.png";

  std::vector<uint8_t> data;
  uint32_t w;
  uint32_t h;
  uint32_t channels = 4;
  uint32_t desired_channels = 4;
  // fpng::fpng_decode_file(app_icon.generic_string().c_str(), data, w, h, channels,
  // desired_channels);

  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data =
      stbi_load(app_icon.generic_string().c_str(), &image_width, &image_height, NULL, 4);
  m_AppIcon = std::make_shared<mcctp::Image>(image_data, image_width, image_height, 4);
  if (image_data != NULL)
    stbi_image_free(image_data);
  */

    using namespace mccinfo::fsm;

    cb_table_.add_callback(ON_STATE_ENTRY | OFF, [&] { mcc_state = "OFF"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | LAUNCHING, [&] { mcc_state = "LAUNCHING"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | ON, [&] { mcc_state = "ON"; }
    );

    cb_table_.add_callback(ON_STATE_ENTRY | OFFLINE, [&] { user_state = "OFFLINE"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | WAITING_ON_LAUNCH, [&] { user_state = "WAITING ON LAUNCH"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | IDENTIFYING_SESSION, [&] { user_state = "IDENTIFYING SESSION"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | IN_MENUS, [&] { user_state = "IN MENUS"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | LOADING_IN, [&] { user_state = "LOADING IN"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | IN_GAME, [&] { user_state = "IN GAME"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | LOADING_OUT, [&] { user_state = "LOADING OUT"; }
    );

    cb_table_.add_callback(ON_STATE_ENTRY | NONE, [&] { game_id_state = "n/a"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALOCE, [&] { game_id_state = "Halo CE"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALO2, [&] { game_id_state = "Halo 2"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALO2A, [&] { game_id_state = "Halo 2 Anniversary"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALO3, [&] { game_id_state = "Halo 3"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALO3ODST, [&] { game_id_state = "Halo 3 ODST"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALO4, [&] { game_id_state = "Halo 4"; }
    );
    cb_table_.add_callback(ON_STATE_ENTRY | HALOREACH, [&] { game_id_state = "Halo Reach"; }
    );

    context_ = std::make_unique<context>(cb_table_);

    context_->start();
}

void Monitor::OnAttach() {
    auto file_xuid = 
        mccinfo::file_readers::GetTheaterFileXUID(
            "C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\Halo3\\copy\\autosave\\asq_constru_21A0C649.temp"
        );

    if (file_xuid.has_value()) {
        theater_file_xuid << file_xuid.value();
    }

    auto file_timestamp = 
        mccinfo::file_readers::GetTheaterFileTimestamp(
            "C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\Halo3\\copy\\autosave\\asq_constru_21A0C649.temp"
        );

    if (file_timestamp.has_value()) {
        theater_file_timestamp << file_timestamp.value();
    }

    auto gametype = 
        mccinfo::file_readers::GetTheaterFileGameType(
            "C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\Halo3\\copy\\autosave\\asq_constru_21A0C649.temp"
    );

    if (gametype.has_value()) {
        theater_file_gametype << gametype.value();
    }

    auto file_desc = 
        mccinfo::file_readers::GetTheaterFileGameDescription(
            "C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\Halo3\\copy\\autosave\\asq_constru_21A0C649.temp"
    );

    if (file_desc.has_value()) {
        theater_file_desc << file_desc.value();
    }
}

void Monitor::OnUpdate(float ts) {

    m_FrameTime = ts;
}

void Monitor::OnUIRender() {
    if (show_demo) {
      ImGui::ShowDemoWindow();
    }

    DoMainMenuBar();

    ImGui::Begin("Monitor");
    ImGui::Text("MCC:");
    ImGui::SameLine();
    ImGui::Text("%s", mcc_state.c_str());
    ImGui::Text("Status:");
    ImGui::SameLine();
    ImGui::Text("%s", user_state.c_str());
    ImGui::SameLine();
    ImGui::Text("(%s)", game_id_state.c_str());
    ImGui::Text("Map:");        
    ImGui::SameLine();
    ImGui::Text("(%s)", context_->get_map_info().c_str());
    ImGui::Text("Theater File XUID:");
    ImGui::SameLine();
    ImGui::Text("(%s)", theater_file_xuid.str().c_str());
    ImGui::Text("Theater File Timestamp:");
    ImGui::SameLine();
    ImGui::Text("(%s) UTC", theater_file_timestamp.str().c_str());
    ImGui::Text("Theater File Gametype:");
    ImGui::SameLine();
    ImGui::TextWrapped("(%s)", theater_file_gametype.str().c_str());
    ImGui::Text("Theater File Desc:");
    ImGui::SameLine();
    ImGui::TextWrapped("(%s)", theater_file_desc.str().c_str());
    ImGui::End();

    DoStatusBar();
}

void Monitor::DoMainMenuBar(void) {
    auto& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x,5 * style.FramePadding.y));
    ImGui::BeginMainMenuBar();
    ImGui::PopStyleVar();
    //m_AppIcon->Bind();
    //ImGui::Image((void *)(intptr_t)m_AppIcon->GetRendererID(), ImVec2(80, 50));
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Show Demo")) {
        show_demo = true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Project")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();
}

void Monitor::DoStatusBar(void)
{
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginViewportSideBar("##MainStatusBar", NULL, ImGuiDir_Down, height, window_flags)) {
        if (ImGui::BeginMenuBar()) {

            float cpu_pct = mccinfo::utils::get_current_process_total_cpu_used();
            size_t bytes = mccinfo::utils::get_current_process_physical_memory();
            size_t total_bytes = mccinfo::utils::get_total_physical_memory();

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f * m_FrameTime,
                1.0f / m_FrameTime);

            ImGui::Text("cpu: (%.1f %)", cpu_pct);
            ImGui::Text("mem: (%.2f MiB/ %.2f GiB)", bytes / (1024.0 * 1024.0), total_bytes / (1024.0 * 1024.0 * 1024.0));

            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}


std::pair<ImVec2, ImVec2> GetScreenFill(void) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImVec2 filled;
    filled.x = pos.x + avail.x;
    filled.y = pos.y + avail.y;

    return {pos, filled};
}

std::pair<ImVec2, ImVec2> GetScreenMaintainAspectRatio(uint32_t width, uint32_t height) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImVec2 aspected = avail;
    float aspect_ratio = (float)width / (float)height;

    aspected.y = aspected.x / aspect_ratio;
    float yOff = (avail.y - aspected.y) / 2;
    if (yOff >= 0.0f) {
      pos.y += yOff;
    } else {
      aspected = avail;
      aspected.x = aspected.y * aspect_ratio;
      float xOff = (avail.x - aspected.x) / 2;
      if (xOff >= 0.0f) {
        pos.x += xOff;
      }
    }
    aspected.x += pos.x;
    aspected.y += pos.y;

    return {pos, aspected};
}
std::pair<ImVec2, ImVec2> GetItemRectMaintainAspectRatio(uint32_t width, uint32_t height, ImVec2 item_extents) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = item_extents;

    ImVec2 aspected = avail;
    float aspect_ratio = (float)width / (float)height;

    aspected.y = aspected.x / aspect_ratio;
    float yOff = (avail.y - aspected.y) / 2;
    if (yOff >= 0.0f) {
        pos.y += yOff;
    }
    else {
        aspected = avail;
        aspected.x = aspected.y * aspect_ratio;
        float xOff = (avail.x - aspected.x) / 2;
        if (xOff >= 0.0f) {
            pos.x += xOff;
        }
    }
    aspected.x += pos.x;
    aspected.y += pos.y;

    return { pos, aspected };
}

}
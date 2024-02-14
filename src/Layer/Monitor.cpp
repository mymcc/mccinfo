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

void Monitor::TestHalo3TheaterFileRead() {
    // halo 3 - customs (good)
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\Halo3\\copy\\autosave\\asq_constru_21A0C649.temp";
    //std::wstring target = L"D:\\dev\\mcctt\\src\\Matches\\b4a9270c-bbdc-9d9e-e066-1df3d53a6590\\asq_bunkerw_2C4F3DF4.film";
    //std::wstring target = L"D:\\dev\\mcctt\\src\\Matches\\a5d790d9-af4f-1f6d-2785-377c59b1021e\\asq_midship_6FA9062F.film";
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\Halo3\\autosave\\asq_guardia_442F8E8F.temp";
    
    // we just need to use a lookup table to identify whether or not the map is campaign or firefight
    //halo 3 odst - firefight (good)
    std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\Halo3ODST\\Movie\\asq_h100_E3C88E28_65CA916B.mov";
    
    //halo3 - campaign tsavo highway (good)
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\Halo3\\Movie\\asq_030_out_6E66FA4F_65CBD633.mov";
    mccinfo::file_readers::halo3_theater_file_reader reader;
    auto file_data_query = reader.Read(target);
    if (file_data_query.has_value()) {
        file_data = file_data_query.value();
        theater_file_timestamp << file_data.utc_timestamp_;
    }
}

void Monitor::TestHaloReachTheaterFileRead() {
    // halo reach - customs (good)
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\HaloReach\\Movie\\asq_mglo-7_forge_hal_3EC4B5D4_65CC2B8A.mov";
    
    // halo reach - campaign long night of solace
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\HaloReach\\Movie\\asq_campaign_m45_F613C570_65CC2CE0.mov";
    
    // halo reach - firefight
    std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\HaloReach\\Movie\\asq_survival_ff45_corv_8E296660_65CC2C77.mov";

    mccinfo::file_readers::haloreach_theater_file_reader reader;
    auto file_data_query = reader.Read(target);
    if (file_data_query.has_value()) {
        file_data = file_data_query.value();
        theater_file_timestamp << file_data.utc_timestamp_;
    }
}

void Monitor::TestHalo4TheaterFileRead() {
    // halo 4 - customs (good)
    std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\Halo4\\Movie\\asq_mglo-1_ca_redoub_C8763617_65CC4085.mov";
    // halo 4 - campaign (good)
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\Halo4\\Movie\\asq_campaign_m80_delta_2798B799_65CC51E7.mov";
    // halo 4 - spartan ops (good)
    //std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\Halo4\\Movie\\asq_firefight_ff87_chop_D5ACD677_65CA95EA.mov";

    mccinfo::file_readers::halo4_theater_file_reader reader;
    auto file_data_query = reader.Read(target);
    if (file_data_query.has_value()) {
        file_data = file_data_query.value();
        theater_file_timestamp << file_data.utc_timestamp_;
    }
}

void Monitor::TestHalo2ATheaterFileRead() {
    // halo 2A - matchmaking
    std::wstring target = L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\UserContent\\Halo2A\\Movie\\asq_mglo-2_ca_sanctu_50AA34A8_65CC5754.mov";

    mccinfo::file_readers::halo2a_theater_file_reader reader;
    auto file_data_query = reader.Read(target);
    if (file_data_query.has_value()) {
        file_data = file_data_query.value();
        theater_file_timestamp << file_data.utc_timestamp_;
    }
}

void Monitor::OnAttach() {
    //TestHalo3TheaterFileRead();
    //TestHaloReachTheaterFileRead();
    //TestHalo4TheaterFileRead();
    TestHalo2ATheaterFileRead();
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
    ImGui::Text("Theater File Author:");
    ImGui::SameLine();
    ImGui::Text("(%s)", file_data.author_.c_str());
    ImGui::Text("Theater File XUID:");
    ImGui::SameLine();
    ImGui::Text("(%s)", file_data.author_xuid_.c_str());
    ImGui::Text("Theater File Timestamp:");
    ImGui::SameLine();
    ImGui::Text("(%s) UTC", theater_file_timestamp.str().c_str());
    ImGui::Text("Theater File Gametype:");
    ImGui::SameLine();
    ImGui::TextWrapped("(%s)", file_data.gametype_.c_str());
    ImGui::Text("Theater File Desc:");
    ImGui::SameLine();
    ImGui::TextWrapped("(%s)", file_data.desc_.c_str());

    ImGui::Text("Players (%i):", file_data.player_set_.size());
    for (const auto &p : file_data.player_set_) {
        ImGui::Text("\t%s (%i)", p.c_str(), p.size());
    }
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
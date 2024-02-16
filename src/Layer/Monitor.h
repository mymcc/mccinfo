#pragma once
#include "Layer.h"
//#include "Graphics/Image.h"

namespace mccinfo {
class Monitor : public Layer {
public:
  Monitor();
  virtual void OnAttach() override;
  virtual void OnUpdate(float ts);
  virtual void OnUIRender() override;

private:
  void DoMainMenuBar(void);
  void DoStatusBar(void);
  void DoTheaterFileInfo(void);

private:
	bool show_demo = false;
	float m_FrameTime = 0.0f;
    std::string mcc_state;
    std::string user_state;
    std::string game_id_state;
    mccinfo::file_readers::game_hint hint;
    mccinfo::fsm::callback_table cb_table_{};
    std::unique_ptr<mccinfo::fsm::context> context_;
};
} // namespace mcctp
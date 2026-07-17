#pragma once

#include "imgui.h"
#include "im_anim.h"

namespace CustomImGui {

inline void BeginAnimationFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    iam_update_begin_frame();
    iam_clip_update(io.DeltaTime);
}

} // namespace CustomImGui

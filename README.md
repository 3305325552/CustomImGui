# CustomImGui

CustomImGui integrates Dear ImGui's docking branch and ImAnim into one reusable CMake package for Windows and DirectX 11 applications.

## Submodules

- `third_party/imgui`: upstream [Dear ImGui](https://github.com/ocornut/imgui) at a pinned docking commit.
- `third_party/ImAnim`: upstream [ImAnim](https://github.com/soufianekhiat/ImAnim) at a pinned commit.

Initialize with:

```powershell
git submodule update --init third_party/imgui third_party/ImAnim
```

The repository records exact submodule commits for reproducible builds. Do not use `git submodule update --remote` in automated builds.

## CMake Usage

```cmake
add_subdirectory(third_party/imgui)
target_link_libraries(your_app PRIVATE CustomImGui::custom_imgui)
```

The package also exposes an `imgui` alias target for projects that already link a target named `imgui`.

## Frame Lifecycle

After `ImGui::NewFrame()`, call:

```cpp
CustomImGui::BeginAnimationFrame();
```

This forwards to ImAnim's required per-frame update calls.

## Test

```powershell
cmake --fresh --preset release
cmake --build --preset release
.\build\release\custom_imgui_dx11_test.exe
```

Pull requests run formatting, clangd diagnostics, and a complete Clang build through the `Quality` workflow.

## License

CustomImGui is available under the [PolyForm Noncommercial License 1.0.0](LICENSE). Noncommercial use is permitted; commercial use is not licensed. Dear ImGui and ImAnim retain their upstream licenses; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

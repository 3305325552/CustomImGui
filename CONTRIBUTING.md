# Contributing

Use a short-lived branch and open a pull request against `main`. Keep changes limited to the integration layer; upstream Dear ImGui and ImAnim changes should be proposed to their respective projects.

Pull requests must pass the `Quality` workflow, which checks formatting, clangd diagnostics, and a complete Clang build of the DX11 smoke application.

By submitting a contribution, you agree to make that contribution available under the project's [PolyForm Noncommercial License 1.0.0](LICENSE).

Update submodules by selecting and testing an exact upstream commit. Do not configure branch-following or `--remote` updates in CI.

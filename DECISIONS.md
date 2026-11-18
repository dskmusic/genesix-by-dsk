# Design decisions

Decisions made where the brief was ambiguous; the most robust option was chosen.

| # | Topic | Decision | Why |
|---|-------|----------|-----|
| 1 | License | **GPLv3** | JUCE 8 is GPLv3/commercial; freeware with public source and no JUCE commercial licence must be GPLv3. VST3 SDK is MIT. |
| 2 | JUCE version | **8.0.17-security** (latest 8.x tag) | Brief asks for JUCE 8; JUCE 9 exists but is out of scope. Pinned tag, fetched with FetchContent or taken from `third_party/JUCE` offline. |
| 3 | Dependency manager | Plain `FetchContent` (no CPM) | Only one dependency (JUCE); CPM adds nothing. |
| 4 | Repo visibility | Public from the first push | Windows Actions minutes are free and unmetered on public repos; the `.gitignore` audit ran before the first commit. |
| 5 | `build_local.sh/.bat` | **Committed** | They are generic (no personal paths); deps dir overridable with `GX_DEPS`. Only `BUILD_LOCAL.md` is ignored. |
| 6 | `prompt inicial.txt` | Ignored | Personal working file. |
| 7 | Windows deps path | `%USERPROFILE%\.gx-deps` | Project path contains spaces and is deep; JUCE object paths would hit MAX_PATH. |
| 8 | `COPY_PLUGIN_AFTER_BUILD` | FALSE | Copying to Common Files\VST3 needs admin rights. |
| 9 | Plugin codes | Manufacturer `Dskm`, plugin `Gnsx` | Frozen forever (DAW session compatibility). |
| 10 | Shared code | One INTERFACE source set compiled into the plugin and into `GenesixTool` | The tool runs unit tests, offline renders, preset validation, factory bank generation, SFZ export and screenshots with the exact same engine. |
| 11 | Commit e-mail | Repo-local GitHub noreply address | The repo is public; keeps the personal e-mail out of commit metadata. |

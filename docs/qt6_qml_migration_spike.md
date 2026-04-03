# Qt 6 / QML Migration Spike

## Scope

This spike adds an optional Qt 6 integration path that proves three things inside the current client:

- the client can host a Qt Quick scene without giving Qt ownership of the main loop;
- gameplay data can be pushed into QML models each frame;
- the Qt scene can be composited into the existing modern overlay texture path.

The production recommendation remains a D3D11-first `QQuickRenderControl` integration that renders directly into a GPU texture owned by the game renderer. The code in this spike uses a temporary CPU image bridge so the event loop, model flow, and composition seams can be validated without rewriting `RenderDevice` yet.

## GDI Ownership Inventory

### QML-owned candidates

- `src/ui/UIWindowMgr.cpp`
  - `UIWindowMgr::OnDraw()` and `UIWindowMgr::OnDrawExcludingRoMap()` own the top-level 2D UI composition order.
- `src/ui/*.cpp`
  - Login, character select, inventories, chat, NPC dialog, option, status, equip, minimap chrome, and other menus are drawn as `UIWindow` subclasses.
- `src/gamemode/GameMode.cpp`
  - `DrawGameplayOverlayToHdc()`, `DrawPlayerVitalsOverlay()`, locked-target overlays, cursor overlays, and modern overlay composition.
- `src/world/MsgEffect.cpp`
  - Damage numbers are still queued as 2D screen draws and painted via GDI helpers.
- `src/ui/NpcDialogColoredText.cpp`
  - Colored NPC dialog text is direct GDI text output.

### Renderer-owned textured quad candidates

- `src/world/World.cpp`
  - `GetPlayerScreenLabel()`, `GetActorScreenMarker()`, `FindHoveredActorScreen()`, and `GetGroundItemScreenMarker()` already expose world-to-screen anchor seams that QML can consume.
- `src/gamemode/GameMode.cpp`
  - `QueueModernOverlayQuad()` already uploads a 2D surface to `CTexture` and submits a full-screen quad.
- `src/render3d/RenderDevice.h`
  - The modern render path already supports texture-backed overlays even though the surface source is currently GDI.

### Deferred non-Qt follow-up work

- `src/world/GameActor.cpp`
  - Actor and item billboards are still rasterized on CPU bitmap surfaces and uploaded to `CTexture`.
- `src/world/RagEffect.cpp`
  - Screen quads and effect billboards remain renderer-side work and should stay there until the UI migration is stable.

## Architecture Decision

### Chosen target architecture

- Keep the game renderer in ownership of the frame.
- Keep the current world render order.
- Replace the GDI overlay stage with a Qt Quick scene rendered offscreen.
- Feed Qt with projected anchor data from engine code instead of duplicating camera math in QML.

### Why this is the chosen direction

- It fits the current `CGameMode::OnRun()` flow, where the 3D scene is built before overlays are queued.
- It keeps world rendering, depth rules, and backend ownership in engine code.
- It gives a clean exit path from `GetDC`, `BitBlt`, `TextOutA`, and `UpdateBackBufferFromMemory` driven UI composition.

### Alternatives considered

- Qt owns the full frame through `QSGRenderNode` or a custom QRhi scene graph item.
  - Rejected for the spike because it would force an immediate renderer inversion and larger backend refactor.
- `QQuickFramebufferObject`.
  - Rejected because Qt documents it as legacy and OpenGL-only.
- Keep a permanent child Qt window layered above the game view.
  - Rejected because it preserves a hybrid window/compositor model instead of converging on renderer-owned composition.

### Prototype compromise in this spike

- The recommended end state is a D3D11 texture target driven by `QQuickRenderControl`.
- The code in this spike uses an offscreen Qt Quick paint-device bridge and composites the resulting image into the existing modern overlay texture path.
- This keeps the spike small while validating the parts that are most risky in this codebase: event loop coexistence, per-frame data binding, and replacing screen-space GDI output with QML output.

## Prototype Implemented Here

- Optional build flag: `RO_ENABLE_QT6_UI_SPIKE`.
- Runtime opt-in: `OPEN_MIDGARD_QT_UI_SPIKE=1`.
- Backend focus: modern path only, with `Direct3D11` treated as the intended target backend for the next phase.
- Data exposed to QML:
  - player screen label anchor;
  - hovered actor label anchor;
  - hovered ground-item label anchor;
  - login status and recent chat preview;
  - last routed mouse and keyboard input seen by the Win32 shell.

## Cutover Phases

1. Keep `RenderDevice` and `CRenderer` in control and replace only the modern overlay source with Qt Quick output.
2. Add a Qt-backed façade beside `g_windowMgr` so gameplay state can drive either legacy `UIWindow` rendering or Qt models during transition.
3. Move damage numbers, hover labels, player vitals, chat preview, and login/character-select panels to QML.
4. Remove `DrawGameplayOverlayToHdc()`, `DrawQueuedMsgEffects()`, `UIWindowMgr::OnDraw()` GDI presentation, and direct `GetDC` / `BitBlt` composition from the gameplay path.
5. Decide case-by-case whether billboard textures in `src/world/GameActor.cpp` stay renderer-owned or become Qt-fed label/sprite items.
6. After the D3D11 path is stable, expand the Qt render target bridge to D3D12 and Vulkan.

## Next Renderer Work

- Teach `RenderDevice` to expose a D3D11 texture seam that can back `QQuickRenderTarget::fromD3D11Texture(...)`.
- Let the Qt spike adopt the engine D3D11 device via `QQuickWindow::setGraphicsDevice(...)`.
- Remove the temporary CPU image bridge once the direct GPU path is stable.

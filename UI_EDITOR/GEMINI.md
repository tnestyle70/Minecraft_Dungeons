# UI EDITOR Project Progress & Roadmap

This file serves as a memory for Gemini CLI to resume the project.

## 📌 Project Overview
A professional UI Editor built with C++, DirectX 9, and ImGui, designed for creating and exporting UI layouts (JSON) compatible with game engines like Unreal.

## ✅ Completed Stages

### Stage 1: Core Foundation & Data Management
- [x] **Entity Expansion:** Added `id`, `visible`, `locked`, and `parentId` to the `Entity` struct.
- [x] **Unique ID System:** Implemented `nextId` to prevent ImGui ID collisions.
- [x] **Save/Load:** Implemented JSON serialization/deserialization compatible with `CanvasPanelSlot`.
- [x] **Editing Productivity:** Added shortcuts for Save (`Ctrl+S`), Duplicate (`Ctrl+D`), and Delete (`Delete`).
- [x] **Property Controls:** Added toggles for `Visible` (hiding on canvas) and `Locked` (preventing accidental drags).

### Stage 2: Parenting System & Relative Coordinates
- [x] **Hierarchical Tree View:** Refactored Hierarchy window to use recursive tree rendering.
- [x] **Drag & Drop Parenting:** Support for changing parent-child relationships via drag and drop in the Hierarchy window.
- [x] **Relative Coordinate System:** Children move seamlessly with parents; local coordinates are automatically recalculated when parenting changes.
- [x] **Global Position Calculation:** Recursive `GetGlobalPos` for accurate world-space rendering.
- [x] **Safety Logic:** Prevented circular references and ensured safe unparenting on parent deletion.

## 🚀 Next Steps (Roadmap)

### Stage 3: Precision Tools & Layout
- [x] **Grid Snapping:** Toggleable snapping to custom grids (8, 16, 32px etc.).
- [x] **Alignment Tools:** Buttons to Align Left/Right/Center/Top/Bottom/Middle relative to canvas.
- [x] **Multi-selection:** Selecting multiple entities using `Ctrl+Click` and multi-dragging.
- [x] **Anchors & Pivots:** Essential for responsive UI layouts added to properties.

### Stage 4: Stability & Advanced UX
- [x] **Undo/Redo System:** Implementation of the Command pattern foundation.
- [x] **Copy/Paste:** Support for `Ctrl+C` and `Ctrl+V` internal clipboard.
- [x] **Search & Filter:** Finding entities in the Hierarchy by name.

---
*Last Updated: 2026-03-13*

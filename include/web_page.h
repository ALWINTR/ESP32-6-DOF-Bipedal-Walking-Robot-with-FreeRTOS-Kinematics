#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Biped Robot - Mission Control Studio</title>
  <style>
    :root {
      --bg-dark: #070b14;
      --bg-card: rgba(15, 23, 42, 0.75);
      --bg-card-hover: rgba(30, 41, 59, 0.85);
      --border-card: rgba(56, 189, 248, 0.16);
      --border-card-active: rgba(56, 189, 248, 0.5);
      --accent-cyan: #06b6d4;
      --accent-sky: #38bdf8;
      --accent-indigo: #6366f1;
      --accent-amber: #f59e0b;
      --accent-emerald: #10b981;
      --accent-rose: #f43f5e;
      --accent-violet: #8b5cf6;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --radius-sm: 8px;
      --radius-md: 14px;
      --radius-lg: 20px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; user-select: none; -webkit-tap-highlight-color: transparent; }
    body {
      background: radial-gradient(circle at 50% 0%, #0d1e38 0%, #070b14 70%);
      color: var(--text-main);
      min-height: 100vh;
      padding: 12px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .container { width: 100%; max-width: 960px; display: flex; flex-direction: column; gap: 14px; }

    /* Top HUD Header */
    header {
      background: var(--bg-card);
      backdrop-filter: blur(14px);
      -webkit-backdrop-filter: blur(14px);
      border: 1px solid var(--border-card);
      border-radius: var(--radius-lg);
      padding: 12px 18px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 10px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.45);
    }
    .brand-box { display: flex; align-items: center; gap: 12px; }
    .brand-icon {
      width: 42px; height: 42px; border-radius: 12px;
      background: linear-gradient(135deg, var(--accent-cyan), var(--accent-indigo));
      display: flex; align-items: center; justify-content: center; font-size: 1.4rem;
      box-shadow: 0 0 18px rgba(6, 182, 212, 0.45);
    }
    .brand-title {
      font-size: 1.15rem; font-weight: 900; letter-spacing: 0.5px;
      background: linear-gradient(135deg, #38bdf8, #818cf8, #c084fc);
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    }
    .brand-sub { font-size: 0.68rem; color: var(--text-muted); font-weight: 700; text-transform: uppercase; letter-spacing: 1px; }
    .hud-badges { display: flex; align-items: center; gap: 8px; flex-wrap: wrap; }
    .hud-badge {
      display: flex; align-items: center; gap: 6px; font-size: 0.72rem; font-weight: 800;
      background: rgba(0,0,0,0.4); padding: 5px 10px; border-radius: 20px; border: 1px solid var(--border-card);
    }
    .dot-pulse { width: 8px; height: 8px; border-radius: 50%; background: var(--accent-emerald); box-shadow: 0 0 10px var(--accent-emerald); animation: pulse 2s infinite; }
    @keyframes pulse { 0%, 100% { opacity: 0.4; } 50% { opacity: 1; } }

    /* Extended System Stats Ribbon */
    .system-ribbon {
      background: rgba(0,0,0,0.35); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 7px 12px;
      display: flex; justify-content: space-between; align-items: center;
      flex-wrap: wrap; gap: 8px; font-size: 0.72rem; font-weight: 700;
    }
    .ribbon-item { display: flex; align-items: center; gap: 5px; color: var(--text-muted); }
    .ribbon-val { font-family: ui-monospace, monospace; font-weight: 900; color: #fff; }

    /* Cards */
    .card {
      background: var(--bg-card);
      backdrop-filter: blur(14px);
      -webkit-backdrop-filter: blur(14px);
      border: 1px solid var(--border-card);
      border-radius: var(--radius-md);
      padding: 15px;
      display: flex;
      flex-direction: column;
      gap: 12px;
      box-shadow: 0 8px 24px rgba(0,0,0,0.3);
    }
    .card-header {
      display: flex; justify-content: space-between; align-items: center;
      border-bottom: 1px solid rgba(255,255,255,0.06); padding-bottom: 9px;
      flex-wrap: wrap; gap: 8px;
    }
    .card-title { font-size: 0.92rem; font-weight: 900; letter-spacing: 0.5px; display: flex; align-items: center; gap: 8px; }
    .tab-pills { display: flex; gap: 6px; }
    .tab-pill {
      background: rgba(255,255,255,0.06); border: 1px solid var(--border-card);
      color: var(--text-muted); font-size: 0.72rem; font-weight: 800; padding: 4px 10px;
      border-radius: 12px; cursor: pointer; transition: all 0.2s;
    }
    .tab-pill.active {
      background: linear-gradient(135deg, rgba(6,182,212,0.3), rgba(99,102,241,0.3));
      border-color: var(--accent-sky); color: #fff; box-shadow: 0 0 10px rgba(6,182,212,0.3);
    }

    /* Motion Commander Deck */
    .motion-grid { display: grid; grid-template-columns: 1fr 1.35fr; gap: 14px; align-items: center; }
    @media (max-width: 680px) { .motion-grid { grid-template-columns: 1fr; } }

    /* D-Pad */
    .dpad-wrapper { display: flex; flex-direction: column; align-items: center; gap: 8px; }
    .dpad {
      display: grid; grid-template-columns: repeat(3, 70px); grid-template-rows: repeat(3, 70px);
      gap: 7px; justify-content: center; margin: auto;
    }
    .dpad-btn {
      background: rgba(255,255,255,0.05); border: 1px solid var(--border-card);
      color: var(--text-main); font-size: 1.25rem; font-weight: 900; border-radius: var(--radius-md);
      display: flex; flex-direction: column; align-items: center; justify-content: center;
      cursor: pointer; transition: all 0.15s;
    }
    .dpad-btn:hover {
      background: rgba(6,182,212,0.22); border-color: var(--accent-cyan);
      color: var(--accent-cyan); box-shadow: 0 0 15px rgba(6,182,212,0.35); transform: translateY(-1px);
    }
    .dpad-btn:active, .dpad-btn.pressed { transform: scale(0.92); background: var(--accent-cyan); color: #000; }
    .dpad-btn span { font-size: 0.58rem; font-weight: 800; text-transform: uppercase; margin-top: 2px; }
    .d-up { grid-column: 2; grid-row: 1; }
    .d-left { grid-column: 1; grid-row: 2; }
    .d-stop {
      grid-column: 2; grid-row: 2; background: rgba(244,63,94,0.22);
      border-color: var(--accent-rose); color: #fda4af; box-shadow: 0 0 12px rgba(244,63,94,0.3);
    }
    .d-stop:hover, .d-stop.pressed { background: var(--accent-rose); color: #fff; }
    .d-right { grid-column: 3; grid-row: 2; }
    .d-down { grid-column: 2; grid-row: 3; }

    /* Action Buttons Box */
    .action-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
    .action-card {
      background: rgba(255,255,255,0.04); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 10px 12px; cursor: pointer;
      display: flex; flex-direction: column; gap: 4px; transition: all 0.15s; text-align: left;
    }
    .action-card:hover {
      background: rgba(255,255,255,0.08); border-color: var(--accent-sky);
      transform: translateY(-1px); box-shadow: 0 4px 12px rgba(0,0,0,0.3);
    }
    .action-card:active { transform: scale(0.97); }
    .action-title { font-size: 0.82rem; font-weight: 800; display: flex; align-items: center; justify-content: space-between; }
    .action-desc { font-size: 0.65rem; color: var(--text-muted); font-weight: 600; }
    .act-walk { border-color: rgba(16,185,129,0.35); background: rgba(16,185,129,0.1); }
    .act-walk .action-title { color: #6ee7b7; }
    .act-step { border-color: rgba(99,102,241,0.35); background: rgba(99,102,241,0.1); }
    .act-step .action-title { color: #a5b4fc; }
    .act-ref { border-color: rgba(139,92,246,0.35); background: rgba(139,92,246,0.1); }
    .act-ref .action-title { color: #c4b5fd; }
    .act-test { border-color: rgba(245,158,11,0.35); background: rgba(245,158,11,0.1); }
    .act-test .action-title { color: #fde68a; }
    .act-stop { border-color: rgba(244,63,94,0.35); background: rgba(244,63,94,0.12); grid-column: span 2; }
    .act-stop .action-title { color: #fca5a5; }

    /* SVG Schematic */
    .schematic-svg {
      width: 100%; max-height: 380px; filter: drop-shadow(0 0 14px rgba(0,0,0,0.5));
      border-radius: 12px; background: radial-gradient(circle at 50% 50%, rgba(15,23,42,0.6) 0%, rgba(6,10,18,0.95) 100%);
      border: 1px solid rgba(255,255,255,0.06);
    }
    .joint-node { cursor: pointer; transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1); }
    .joint-node:hover circle.joint-outer { stroke-width: 3.5px; filter: brightness(1.3); }
    .joint-node.selected circle.joint-outer { stroke-width: 4px; stroke: #fff !important; filter: drop-shadow(0 0 8px #fff); }

    /* Nudge Toolbar */
    .nudge-toolbar {
      width: 100%; background: rgba(0,0,0,0.4); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 8px 12px; display: flex; flex-wrap: wrap;
      justify-content: space-between; align-items: center; gap: 8px; margin-top: 4px;
    }
    .nudge-info { display: flex; align-items: center; gap: 8px; font-size: 0.8rem; }
    .nudge-badge {
      background: rgba(56,189,248,0.16); border: 1px solid var(--accent-sky);
      color: var(--accent-sky); font-weight: 800; padding: 2px 8px; border-radius: 6px;
    }
    .nudge-buttons { display: flex; gap: 5px; flex-wrap: wrap; }
    .nudge-btn {
      background: rgba(255,255,255,0.07); border: 1px solid var(--border-card);
      color: var(--text-main); font-size: 0.75rem; font-weight: 800; padding: 5px 9px;
      border-radius: 6px; cursor: pointer; transition: all 0.15s;
    }
    .nudge-btn:hover { background: var(--accent-sky); color: #000; border-color: var(--accent-sky); }
    .nudge-btn:active { transform: scale(0.92); }
    .nudge-home { background: rgba(16,185,129,0.18); border-color: var(--accent-emerald); color: #6ee7b7; }
    .nudge-home:hover { background: var(--accent-emerald); color: #000; }

    /* Real-Time Joint Oscilloscope Canvas */
    .scope-box {
      width: 100%; height: 180px; position: relative;
      background: #050811; border-radius: var(--radius-sm); border: 1px solid rgba(56,189,248,0.2);
      overflow: hidden; box-shadow: inset 0 0 20px rgba(0,0,0,0.8);
    }
    #scope-canvas { width: 100%; height: 100%; display: block; }
    .scope-legend {
      position: absolute; top: 8px; right: 10px; display: flex; gap: 10px;
      background: rgba(0,0,0,0.65); padding: 4px 8px; border-radius: 6px; font-size: 0.68rem; font-weight: 800;
    }
    .legend-tag { display: flex; align-items: center; gap: 4px; }
    .legend-dot { width: 8px; height: 8px; border-radius: 50%; }

    /* Interactive Web Terminal */
    .terminal-container {
      background: #030712; border: 1px solid rgba(255,255,255,0.08);
      border-radius: var(--radius-sm); overflow: hidden; display: flex; flex-direction: column;
    }
    .terminal-output {
      height: 130px; overflow-y: auto; padding: 10px; font-family: ui-monospace, monospace;
      font-size: 0.75rem; color: #38bdf8; line-height: 1.45; display: flex; flex-direction: column; gap: 3px;
    }
    .log-line { display: flex; gap: 8px; word-break: break-all; }
    .log-time { color: var(--text-muted); font-size: 0.68rem; }
    .terminal-input-bar {
      display: flex; border-top: 1px solid rgba(255,255,255,0.06); background: rgba(0,0,0,0.3);
    }
    .term-prompt { padding: 8px 10px; color: var(--accent-emerald); font-weight: 900; font-family: ui-monospace, monospace; }
    .term-input {
      flex: 1; background: transparent; border: none; outline: none; color: #fff;
      font-family: ui-monospace, monospace; font-size: 0.8rem; padding: 8px 0;
    }
    .term-btn {
      background: rgba(56,189,248,0.15); border: none; border-left: 1px solid var(--border-card);
      color: var(--accent-sky); font-weight: 800; font-size: 0.72rem; padding: 0 14px; cursor: pointer;
    }
    .term-chips { display: flex; gap: 6px; padding: 6px 10px; background: rgba(0,0,0,0.5); flex-wrap: wrap; }
    .term-chip {
      background: rgba(255,255,255,0.06); border: 1px solid var(--border-card); color: var(--text-muted);
      font-size: 0.66rem; font-weight: 800; padding: 2px 7px; border-radius: 4px; cursor: pointer;
    }
    .term-chip:hover { color: var(--accent-sky); border-color: var(--accent-sky); }

    /* Motion Choreographer & Routines */
    .stunt-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(190px, 1fr)); gap: 8px; width: 100%; }
    .stunt-card {
      background: rgba(255,255,255,0.03); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 10px; cursor: pointer; display: flex; flex-direction: column;
      gap: 4px; transition: all 0.15s;
    }
    .stunt-card:hover {
      background: rgba(255,255,255,0.07); border-color: var(--accent-violet); transform: translateY(-1px);
    }
    .stunt-title { font-size: 0.82rem; font-weight: 800; display: flex; align-items: center; justify-content: space-between; color: #c4b5fd; }
    .stunt-desc { font-size: 0.66rem; color: var(--text-muted); }

    /* Keyboard Hotkeys HUD */
    .keys-hud { display: flex; gap: 6px; flex-wrap: wrap; align-items: center; }
    .key-badge {
      background: rgba(255,255,255,0.08); border: 1px solid rgba(255,255,255,0.15);
      border-radius: 5px; padding: 3px 6px; font-size: 0.65rem; font-weight: 900;
      font-family: ui-monospace, monospace; color: var(--text-muted); transition: all 0.1s;
    }
    .key-badge.pressed { background: var(--accent-sky); color: #000; border-color: var(--accent-sky); transform: translateY(1px); }

    /* Telemetry Grid */
    .telemetry-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(130px, 1fr)); gap: 8px; width: 100%; }
    .telem-card {
      background: rgba(0,0,0,0.32); border: 1px solid rgba(255,255,255,0.06);
      border-radius: var(--radius-sm); padding: 9px; display: flex; flex-direction: column;
      align-items: center; gap: 4px; cursor: pointer; transition: all 0.15s;
    }
    .telem-card:hover { border-color: var(--border-card-active); background: rgba(0,0,0,0.48); }
    .telem-name { font-size: 0.7rem; font-weight: 800; color: var(--text-muted); text-transform: uppercase; }
    .telem-angle { font-size: 1.3rem; font-weight: 900; font-family: ui-monospace, monospace; }
    .telem-r { color: var(--accent-amber); }
    .telem-l { color: var(--accent-cyan); }
    .telem-bar-track { width: 100%; height: 4px; background: rgba(255,255,255,0.08); border-radius: 2px; overflow: hidden; }
    .telem-bar-fill { height: 100%; width: 50%; border-radius: 2px; transition: width 0.15s; }

    /* Ground Weight & Reaction Force HUD */
    .weight-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; width: 100%; }
    @media (max-width: 600px) { .weight-grid { grid-template-columns: 1fr; } }
    .weight-card {
      background: rgba(0,0,0,0.32); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 12px 14px; display: flex; flex-direction: column; gap: 7px;
    }
    .weight-card-l { border-color: rgba(6, 182, 212, 0.35); }
    .weight-card-r { border-color: rgba(245, 158, 11, 0.35); }
    .weight-title-row { display: flex; justify-content: space-between; align-items: center; }
    .weight-title { font-size: 0.8rem; font-weight: 800; text-transform: uppercase; letter-spacing: 0.5px; }
    .weight-val { font-size: 1.5rem; font-weight: 900; font-family: ui-monospace, monospace; }
    .weight-bar-bg { width: 100%; height: 9px; background: rgba(255,255,255,0.08); border-radius: 5px; overflow: hidden; }
    .weight-bar-fill { height: 100%; border-radius: 5px; transition: width 0.12s ease-out; }
    .grf-summary-bar {
      display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 8px;
      padding: 9px 13px; background: rgba(0,0,0,0.36); border-radius: var(--radius-sm); border: 1px solid var(--border-card);
    }

    /* Joint Control Cards */
    .leg-section-title {
      font-size: 0.82rem; font-weight: 800; padding: 7px 12px; border-radius: 8px;
      width: 100%; display: flex; justify-content: space-between; align-items: center;
    }
    .leg-r-header { background: rgba(245, 158, 11, 0.14); color: var(--accent-amber); border-left: 4px solid var(--accent-amber); }
    .leg-l-header { background: rgba(6, 182, 212, 0.14); color: var(--accent-cyan); border-left: 4px solid var(--accent-cyan); }
    .servo-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 10px; width: 100%; }
    .servo-card {
      background: rgba(0,0,0,0.25); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 12px; display: flex; flex-direction: column;
      gap: 7px; transition: border-color 0.2s;
    }
    .servo-card.active-target { border-color: var(--accent-sky); box-shadow: 0 0 12px rgba(56,189,248,0.2); }
    .servo-header { display: flex; justify-content: space-between; align-items: center; }
    .servo-name { font-size: 0.82rem; font-weight: 800; }
    .servo-angle-box { display: flex; align-items: baseline; gap: 6px; }
    .servo-angle-text { font-size: 1.05rem; font-weight: 900; font-family: ui-monospace, monospace; }
    .servo-home-tag { font-size: 0.68rem; color: var(--text-muted); font-weight: 700; }
    .slider-styled {
      width: 100%; height: 6px; accent-color: var(--accent-cyan); cursor: pointer;
      background: rgba(255,255,255,0.1); border-radius: 3px;
    }
    .servo-card-r .slider-styled { accent-color: var(--accent-amber); }
    .step-btn-group { display: grid; grid-template-columns: repeat(7, 1fr); gap: 4px; }
    .step-btn {
      background: rgba(255,255,255,0.06); border: 1px solid var(--border-card);
      color: var(--text-muted); font-size: 0.7rem; padding: 5px 2px; border-radius: 4px;
      cursor: pointer; font-weight: 800; text-align: center;
    }
    .step-btn:hover { background: var(--accent-sky); color: #000; }
    .step-btn:active { transform: scale(0.92); }
    .btn-reset-single { background: rgba(16,185,129,0.15); border-color: var(--accent-emerald); color: #6ee7b7; font-size: 0.65rem; }
    .btn-reset-single:hover { background: var(--accent-emerald); color: #000; }

    /* Tuner Sliders */
    .tuner-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); gap: 8px; width: 100%; }
    .tuner-card {
      background: rgba(0,0,0,0.25); border: 1px solid var(--border-card);
      border-radius: var(--radius-sm); padding: 10px; display: flex; flex-direction: column; gap: 6px;
    }
    .tuner-header { display: flex; justify-content: space-between; align-items: center; }
    .tuner-label { font-size: 0.75rem; font-weight: 700; color: var(--text-muted); }
    .tuner-val { font-size: 0.9rem; font-weight: 900; color: var(--accent-sky); font-family: ui-monospace, monospace; }

    /* Footer System Actions */
    .sys-actions { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 8px; width: 100%; }
    .btn-sys {
      background: rgba(255,255,255,0.05); border: 1px solid var(--border-card);
      color: var(--text-main); font-weight: 800; padding: 10px; border-radius: var(--radius-sm);
      cursor: pointer; font-size: 0.78rem; display: flex; align-items: center; justify-content: center; gap: 6px;
      transition: all 0.15s;
    }
    .btn-sys:active { transform: scale(0.98); }
    .btn-save { border-color: var(--accent-emerald); background: rgba(16,185,129,0.15); color: #6ee7b7; }
    .btn-save:hover { background: var(--accent-emerald); color: #000; }
    .btn-restore { border-color: var(--accent-rose); background: rgba(244,63,94,0.15); color: #fda4af; }
    .btn-restore:hover { background: var(--accent-rose); color: #fff; }
    .btn-scan { border-color: var(--accent-sky); background: rgba(56,189,248,0.15); color: #7dd3fc; }
    .btn-scan:hover { background: var(--accent-sky); color: #000; }
    .btn-export { border-color: var(--accent-violet); background: rgba(139,92,246,0.15); color: #c4b5fd; }
    .btn-export:hover { background: var(--accent-violet); color: #000; }

    /* Toast Notification */
    #toast {
      position: fixed; bottom: 20px; right: 20px; z-index: 9999;
      background: rgba(15,23,42,0.95); backdrop-filter: blur(12px);
      border: 1px solid var(--accent-sky); border-radius: 12px;
      padding: 10px 18px; font-size: 0.85rem; font-weight: 800; color: #fff;
      box-shadow: 0 10px 30px rgba(0,0,0,0.6); display: none; align-items: center; gap: 8px;
      transform: translateY(20px); transition: all 0.25s ease;
    }
    #toast.show { display: flex; transform: translateY(0); }
  </style>
</head>
<body>
  <div class="container">
    <!-- Top HUD Header -->
    <header>
      <div class="brand-box">
        <div class="brand-icon">🤖</div>
        <div>
          <div class="brand-title">6-DOF BIPED ROBOT</div>
          <div class="brand-sub">MISSION CONTROL & KINEMATICS STUDIO</div>
        </div>
      </div>
      <div class="hud-badges">
        <div class="hud-badge">
          <div class="dot-pulse"></div>
          <span id="status-text">ONLINE</span>
        </div>
        <div class="hud-badge" id="hud-pca-badge" style="color: var(--accent-sky);">
          <span id="hud-pca-status">PCA9685: CHECKING...</span>
        </div>
        <div class="hud-badge" style="color: var(--accent-emerald);">
          <span id="hud-active-gait">STAND</span>
        </div>
        <div class="hud-badge" id="hud-ping-badge" style="color: var(--accent-cyan);">
          <span>Ping: <span id="hud-ping">--</span>ms</span>
        </div>
      </div>
    </header>

    <!-- Extended System Diagnostics Ribbon -->
    <div class="system-ribbon">
      <div class="ribbon-item">
        <span>💾 Free RAM:</span>
        <span class="ribbon-val" id="stat-heap">-- KB</span>
      </div>
      <div class="ribbon-item">
        <span>⏱️ Uptime:</span>
        <span class="ribbon-val" id="stat-uptime">00:00:00</span>
      </div>
      <div class="ribbon-item">
        <span>📶 WiFi RSSI:</span>
        <span class="ribbon-val" id="stat-rssi">-- dBm</span>
      </div>
      <div class="ribbon-item">
        <span>🌐 Host IP:</span>
        <span class="ribbon-val" id="stat-ip">192.168.4.1</span>
      </div>
      <div class="ribbon-item">
        <span>🎮 Gamepad:</span>
        <span class="ribbon-val" id="stat-gamepad" style="color: var(--text-muted);">DISCONNECTED</span>
      </div>
    </div>

    <!-- Live Motion Oscilloscope (Canvas Waveform) -->
    <section class="card">
      <div class="card-header">
        <div class="card-title">
          <span>📈</span>
          <span>REAL-TIME JOINT TRAJECTORY OSCILLOSCOPE</span>
        </div>
        <div style="display: flex; gap: 8px; align-items: center;">
          <button class="tab-pill" id="btn-scope-pause" onclick="toggleScope()">Pause</button>
          <button class="tab-pill" onclick="clearScope()">Clear</button>
        </div>
      </div>
      <div class="scope-box">
        <canvas id="scope-canvas"></canvas>
        <div class="scope-legend">
          <div class="legend-tag"><div class="legend-dot" style="background: #f59e0b;"></div><span>R.Hip</span></div>
          <div class="legend-tag"><div class="legend-dot" style="background: #06b6d4;"></div><span>L.Hip</span></div>
          <div class="legend-tag"><div class="legend-dot" style="background: #eab308;"></div><span>R.Knee</span></div>
          <div class="legend-tag"><div class="legend-dot" style="background: #3b82f6;"></div><span>L.Knee</span></div>
        </div>
      </div>
    </section>

    <!-- Interactive Robot Movement Schematic Card -->
    <section class="card">
      <div class="card-header">
        <div class="card-title">
          <span>📐</span>
          <span>LIVE ANIMATED ROBOT MOVEMENT & KINEMATICS</span>
        </div>
        <div class="tab-pills">
          <button class="tab-pill active" id="tab-btn-side" onclick="switchSchematic('side')">Side Walk View</button>
          <button class="tab-pill" id="tab-btn-kinematic" onclick="switchSchematic('kinematic')">Front View</button>
          <button class="tab-pill" id="tab-btn-circuit" onclick="switchSchematic('circuit')">Circuit Pinout</button>
        </div>
      </div>

      <!-- Schematic Container -->
      <div style="width: 100%; display: flex; flex-direction: column; align-items: center;">
        
        <!-- 1. FRONT ANIMATED KINEMATIC RIG -->
        <svg id="svg-kinematic" class="schematic-svg" viewBox="0 0 540 370" style="display: none;">
          <defs>
            <linearGradient id="grad-r-link" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%" stop-color="#f59e0b" stop-opacity="0.9" />
              <stop offset="100%" stop-color="#d97706" stop-opacity="0.7" />
            </linearGradient>
            <linearGradient id="grad-l-link" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%" stop-color="#06b6d4" stop-opacity="0.9" />
              <stop offset="100%" stop-color="#0284c7" stop-opacity="0.7" />
            </linearGradient>
            <filter id="glow-amber" x="-20%" y="-20%" width="140%" height="140%">
              <feGaussianBlur stdDeviation="4" result="blur" />
              <feComposite in="SourceGraphic" in2="blur" operator="over" />
            </filter>
            <filter id="glow-cyan" x="-20%" y="-20%" width="140%" height="140%">
              <feGaussianBlur stdDeviation="4" result="blur" />
              <feComposite in="SourceGraphic" in2="blur" operator="over" />
            </filter>
          </defs>

          <!-- Grid Background lines -->
          <line x1="40" y1="90" x2="500" y2="90" stroke="rgba(255,255,255,0.04)" stroke-dasharray="4" />
          <line x1="40" y1="190" x2="500" y2="190" stroke="rgba(255,255,255,0.04)" stroke-dasharray="4" />
          <line x1="40" y1="290" x2="500" y2="290" stroke="rgba(255,255,255,0.04)" stroke-dasharray="4" />

          <!-- Center Pelvis / Controller Core -->
          <rect x="200" y="20" width="140" height="48" rx="8" fill="#1e293b" stroke="#38bdf8" stroke-width="2" />
          <text x="270" y="40" text-anchor="middle" fill="#38bdf8" font-size="11" font-weight="900">ESP32 + PCA9685</text>
          <text x="270" y="55" text-anchor="middle" fill="#94a3b8" font-size="9" font-weight="700">PELVIS CORE (0x40)</text>

          <!-- Pelvis to Hip Structural Brackets -->
          <path d="M 220 68 L 140 100" stroke="#f59e0b" stroke-width="3" stroke-linecap="round" />
          <path d="M 320 68 L 400 100" stroke="#06b6d4" stroke-width="3" stroke-linecap="round" />

          <!-- Right Leg Links -->
          <line id="link-thigh-r" x1="140" y1="100" x2="140" y2="200" stroke="url(#grad-r-link)" stroke-width="6" stroke-linecap="round" />
          <line id="link-shin-r" x1="140" y1="200" x2="140" y2="300" stroke="url(#grad-r-link)" stroke-width="6" stroke-linecap="round" />
          <rect id="plate-foot-r" x="105" y="318" width="70" height="14" rx="4" fill="#f59e0b" opacity="0.85" />
          <text id="plate-text-r" x="140" y="329" text-anchor="middle" fill="#000" font-size="8" font-weight="900">R.FOOT SOLE</text>

          <!-- Left Leg Links -->
          <line id="link-thigh-l" x1="400" y1="100" x2="400" y2="200" stroke="url(#grad-l-link)" stroke-width="6" stroke-linecap="round" />
          <line id="link-shin-l" x1="400" y1="200" x2="400" y2="300" stroke="url(#grad-l-link)" stroke-width="6" stroke-linecap="round" />
          <rect id="plate-foot-l" x="365" y="318" width="70" height="14" rx="4" fill="#06b6d4" opacity="0.85" />
          <text id="plate-text-l" x="400" y="329" text-anchor="middle" fill="#000" font-size="8" font-weight="900">L.FOOT SOLE</text>

          <!-- Joint Nodes (Right Leg) -->
          <g class="joint-node selected" id="node-0" onclick="selectJoint(0, 'Right Hip')">
            <circle class="joint-outer" cx="140" cy="100" r="22" fill="#0f172a" stroke="#f59e0b" stroke-width="2.5" filter="url(#glow-amber)" />
            <text x="140" y="96" text-anchor="middle" fill="#f59e0b" font-size="9" font-weight="800">CH 0</text>
            <text id="svg-angle-0" x="140" y="110" text-anchor="middle" fill="#fff" font-size="11" font-weight="900">94°</text>
            <text x="80" y="104" text-anchor="middle" fill="#f59e0b" font-size="10" font-weight="700">R.HIP</text>
          </g>
          <g class="joint-node" id="node-1" onclick="selectJoint(1, 'Right Knee')">
            <circle class="joint-outer" cx="140" cy="200" r="22" fill="#0f172a" stroke="#f59e0b" stroke-width="2.5" filter="url(#glow-amber)" />
            <text x="140" y="196" text-anchor="middle" fill="#f59e0b" font-size="9" font-weight="800">CH 1</text>
            <text id="svg-angle-1" x="140" y="210" text-anchor="middle" fill="#fff" font-size="11" font-weight="900">8°</text>
            <text x="80" y="204" text-anchor="middle" fill="#f59e0b" font-size="10" font-weight="700">R.KNEE</text>
          </g>
          <g class="joint-node" id="node-12" onclick="selectJoint(12, 'Right Foot')">
            <circle class="joint-outer" cx="140" cy="300" r="22" fill="#0f172a" stroke="#f59e0b" stroke-width="2.5" filter="url(#glow-amber)" />
            <text x="140" y="296" text-anchor="middle" fill="#f59e0b" font-size="9" font-weight="800">CH 12</text>
            <text id="svg-angle-12" x="140" y="310" text-anchor="middle" fill="#fff" font-size="11" font-weight="900">94°</text>
            <text x="80" y="304" text-anchor="middle" fill="#f59e0b" font-size="10" font-weight="700">R.FOOT</text>
          </g>

          <!-- Joint Nodes (Left Leg) -->
          <g class="joint-node" id="node-4" onclick="selectJoint(4, 'Left Hip')">
            <circle class="joint-outer" cx="400" cy="100" r="22" fill="#0f172a" stroke="#06b6d4" stroke-width="2.5" filter="url(#glow-cyan)" />
            <text x="400" y="96" text-anchor="middle" fill="#06b6d4" font-size="9" font-weight="800">CH 4</text>
            <text id="svg-angle-4" x="400" y="110" text-anchor="middle" fill="#fff" font-size="11" font-weight="900">94°</text>
            <text x="460" y="104" text-anchor="middle" fill="#06b6d4" font-size="10" font-weight="700">L.HIP</text>
          </g>
          <g class="joint-node" id="node-8" onclick="selectJoint(8, 'Left Knee')">
            <circle class="joint-outer" cx="400" cy="200" r="22" fill="#0f172a" stroke="#06b6d4" stroke-width="2.5" filter="url(#glow-cyan)" />
            <text x="400" y="196" text-anchor="middle" fill="#06b6d4" font-size="9" font-weight="800">CH 8</text>
            <text id="svg-angle-8" x="400" y="210" text-anchor="middle" fill="#fff" font-size="11" font-weight="900">94°</text>
            <text x="460" y="204" text-anchor="middle" fill="#06b6d4" font-size="10" font-weight="700">L.KNEE</text>
          </g>
          <g class="joint-node" id="node-15" onclick="selectJoint(15, 'Left Foot')">
            <circle class="joint-outer" cx="400" cy="300" r="22" fill="#0f172a" stroke="#06b6d4" stroke-width="2.5" filter="url(#glow-cyan)" />
            <text x="400" y="296" text-anchor="middle" fill="#06b6d4" font-size="9" font-weight="800">CH 15</text>
            <text id="svg-angle-15" x="400" y="310" text-anchor="middle" fill="#fff" font-size="11" font-weight="900">94°</text>
            <text x="460" y="304" text-anchor="middle" fill="#06b6d4" font-size="10" font-weight="700">L.FOOT</text>
          </g>

          <!-- Center Status Card in SVG -->
          <rect x="210" y="150" width="120" height="65" rx="6" fill="rgba(15,23,42,0.88)" stroke="rgba(255,255,255,0.12)" />
          <text x="270" y="172" text-anchor="middle" fill="#94a3b8" font-size="9" font-weight="700">GAIT STATE</text>
          <text id="svg-active-gait" x="270" y="195" text-anchor="middle" fill="#10b981" font-size="14" font-weight="900">STAND</text>
        </svg>

        <!-- 2. SIDE SAGITTAL WALKING RIG (Interactive 2D Biped Walking Visualizer) -->
        <svg id="svg-side" class="schematic-svg" viewBox="0 0 540 370" style="display: block;">
          <!-- Ground Surface -->
          <line x1="20" y1="330" x2="520" y2="330" stroke="#38bdf8" stroke-width="2.5" stroke-dasharray="6" />
          <rect x="20" y="332" width="500" height="24" fill="rgba(15,23,42,0.6)" />
          <text x="270" y="348" text-anchor="middle" fill="#64748b" font-size="9" font-weight="700">SAGITTAL SURFACE CONTACT PLANE (GROUND CONTACT: 10.0cm)</text>

          <!-- Torso & Pelvis -->
          <rect x="245" y="20" width="50" height="60" rx="8" fill="#1e293b" stroke="#38bdf8" stroke-width="2" />
          <rect x="255" y="5" width="30" height="18" rx="4" fill="#0f172a" stroke="#38bdf8" stroke-width="1.5" />
          <line x1="260" y1="14" x2="280" y2="14" stroke="#06b6d4" stroke-width="3" stroke-linecap="round" />
          <circle cx="270" cy="50" r="10" fill="none" stroke="#38bdf8" stroke-width="2" stroke-dasharray="3" />
          <text x="270" y="53" text-anchor="middle" fill="#38bdf8" font-size="8" font-weight="900">CORE</text>

          <!-- Hip Axis Origin -->
          <circle cx="270" cy="80" r="10" fill="#0f172a" stroke="#fff" stroke-width="2.5" />
          <text x="270" y="83" text-anchor="middle" fill="#fff" font-size="8" font-weight="900">HIP</text>

          <!-- Left Leg (Cyan Background Layer) -->
          <line id="side-thigh-l" x1="270" y1="80" x2="270" y2="190" stroke="#06b6d4" stroke-width="7" stroke-linecap="round" />
          <circle id="side-knee-l" cx="270" cy="190" r="9" fill="#0f172a" stroke="#06b6d4" stroke-width="2.5" />
          <line id="side-shin-l" x1="270" y1="190" x2="270" y2="310" stroke="#06b6d4" stroke-width="7" stroke-linecap="round" />
          <circle id="side-ankle-l" cx="270" cy="310" r="7" fill="#0f172a" stroke="#06b6d4" stroke-width="2" />
          <rect id="side-foot-l" x="238" y="316" width="64" height="14" rx="4" fill="#06b6d4" opacity="0.9" />
          <text id="side-foot-text-l" x="270" y="327" text-anchor="middle" fill="#000" font-size="8" font-weight="900">L.FOOT (CH15)</text>

          <!-- Right Leg (Amber Foreground Layer) -->
          <line id="side-thigh-r" x1="270" y1="80" x2="270" y2="190" stroke="#f59e0b" stroke-width="7" stroke-linecap="round" />
          <circle id="side-knee-r" cx="270" cy="190" r="9" fill="#0f172a" stroke="#f59e0b" stroke-width="2.5" />
          <line id="side-shin-r" x1="270" y1="190" x2="270" y2="310" stroke="#f59e0b" stroke-width="7" stroke-linecap="round" />
          <circle id="side-ankle-r" cx="270" cy="310" r="7" fill="#0f172a" stroke="#f59e0b" stroke-width="2" />
          <rect id="side-foot-r" x="238" y="316" width="64" height="14" rx="4" fill="#f59e0b" opacity="0.95" />
          <text id="side-foot-text-r" x="270" y="327" text-anchor="middle" fill="#000" font-size="8" font-weight="900">R.FOOT (CH12)</text>

          <!-- Gait Status Badge in Side View -->
          <rect x="390" y="20" width="125" height="55" rx="6" fill="rgba(15,23,42,0.85)" stroke="rgba(255,255,255,0.12)" />
          <text x="452" y="38" text-anchor="middle" fill="#94a3b8" font-size="9" font-weight="800">WALKING GAIT</text>
          <text id="side-gait-status" x="452" y="58" text-anchor="middle" fill="#10b981" font-size="13" font-weight="900">STAND</text>
        </svg>

        <!-- 3. CIRCUIT PINOUT SCHEMATIC SVG -->
        <svg id="svg-circuit" class="schematic-svg" viewBox="0 0 540 370" style="display: none;">
          <rect x="30" y="40" width="140" height="290" rx="8" fill="#1e293b" stroke="#38bdf8" stroke-width="2" />
          <text x="100" y="65" text-anchor="middle" fill="#38bdf8" font-size="12" font-weight="900">ESP32 DevKit</text>
          <text x="100" y="80" text-anchor="middle" fill="#64748b" font-size="8" font-weight="600">30-PIN MCU</text>
          <rect x="135" y="110" width="30" height="18" rx="3" fill="#0284c7" />
          <text x="150" y="123" text-anchor="middle" fill="#fff" font-size="8" font-weight="800">G21 (SDA)</text>
          <rect x="135" y="145" width="30" height="18" rx="3" fill="#0284c7" />
          <text x="150" y="158" text-anchor="middle" fill="#fff" font-size="8" font-weight="800">G22 (SCL)</text>
          <rect x="135" y="190" width="30" height="18" rx="3" fill="#dc2626" />
          <text x="150" y="203" text-anchor="middle" fill="#fff" font-size="8" font-weight="800">3V3</text>
          <rect x="135" y="225" width="30" height="18" rx="3" fill="#475569" />
          <text x="150" y="238" text-anchor="middle" fill="#fff" font-size="8" font-weight="800">GND</text>

          <rect x="230" y="40" width="140" height="290" rx="8" fill="#0f172a" stroke="#6366f1" stroke-width="2" />
          <text x="300" y="65" text-anchor="middle" fill="#818cf8" font-size="12" font-weight="900">PCA9685 Driver</text>
          <text x="300" y="80" text-anchor="middle" fill="#64748b" font-size="8" font-weight="600">I2C: 0x40 (50Hz PWM)</text>
          <path d="M 165 119 L 230 119" stroke="#38bdf8" stroke-width="2.5" />
          <path d="M 165 154 L 230 154" stroke="#818cf8" stroke-width="2.5" />
          <path d="M 165 199 L 230 199" stroke="#ef4444" stroke-width="2" />
          <path d="M 165 234 L 230 234" stroke="#64748b" stroke-width="2" />

          <rect x="250" y="290" width="100" height="30" rx="4" fill="#334155" stroke="#f59e0b" />
          <text x="300" y="309" text-anchor="middle" fill="#fbbf24" font-size="9" font-weight="900">EXT 5V-6V (V+ / GND)</text>

          <g><path d="M 370 100 L 415 100" stroke="#f59e0b" stroke-width="2" /><rect x="415" y="90" width="100" height="22" rx="4" fill="#1e293b" stroke="#f59e0b" /><text x="465" y="104" text-anchor="middle" fill="#fbbf24" font-size="8" font-weight="800">CH 0: Right Hip</text></g>
          <g><path d="M 370 135 L 415 135" stroke="#f59e0b" stroke-width="2" /><rect x="415" y="125" width="100" height="22" rx="4" fill="#1e293b" stroke="#f59e0b" /><text x="465" y="139" text-anchor="middle" fill="#fbbf24" font-size="8" font-weight="800">CH 1: Right Knee</text></g>
          <g><path d="M 370 170 L 415 170" stroke="#f59e0b" stroke-width="2" /><rect x="415" y="160" width="100" height="22" rx="4" fill="#1e293b" stroke="#f59e0b" /><text x="465" y="174" text-anchor="middle" fill="#fbbf24" font-size="8" font-weight="800">CH 12: Right Foot</text></g>
          <g><path d="M 370 215 L 415 215" stroke="#06b6d4" stroke-width="2" /><rect x="415" y="205" width="100" height="22" rx="4" fill="#1e293b" stroke="#06b6d4" /><text x="465" y="219" text-anchor="middle" fill="#67e8f9" font-size="8" font-weight="800">CH 4: Left Hip</text></g>
          <g><path d="M 370 250 L 415 250" stroke="#06b6d4" stroke-width="2" /><rect x="415" y="240" width="100" height="22" rx="4" fill="#1e293b" stroke="#06b6d4" /><text x="465" y="254" text-anchor="middle" fill="#67e8f9" font-size="8" font-weight="800">CH 8: Left Knee</text></g>
          <g><path d="M 370 285 L 415 285" stroke="#06b6d4" stroke-width="2" /><rect x="415" y="275" width="100" height="22" rx="4" fill="#1e293b" stroke="#06b6d4" /><text x="465" y="289" text-anchor="middle" fill="#67e8f9" font-size="8" font-weight="800">CH 15: Left Foot</text></g>
        </svg>

        <!-- Quick Nudge Toolbar -->
        <div class="nudge-toolbar">
          <div class="nudge-info">
            <span style="color: var(--text-muted);">ACTIVE JOINT:</span>
            <span class="nudge-badge" id="selected-joint-badge">Right Hip (CH 0)</span>
            <span style="font-family: monospace; font-weight: 900; font-size: 0.95rem;" id="selected-joint-angle">94°</span>
          </div>
          <div class="nudge-buttons">
            <button class="nudge-btn" onclick="stepSelected(-10)">-10°</button>
            <button class="nudge-btn" onclick="stepSelected(-5)">-5°</button>
            <button class="nudge-btn" onclick="stepSelected(-1)">-1°</button>
            <button class="nudge-btn nudge-home" onclick="homeSelected()">HOME</button>
            <button class="nudge-btn" onclick="stepSelected(+1)">+1°</button>
            <button class="nudge-btn" onclick="stepSelected(+5)">+5°</button>
            <button class="nudge-btn" onclick="stepSelected(+10)">+10°</button>
          </div>
        </div>
      </div>
    </section>

    <!-- Motion Commander Deck -->
    <section class="card">
      <div class="card-header">
        <div class="card-title"><span>🎮</span><span>MOTION COMMANDER & GAIT CONTROL</span></div>
        <div class="keys-hud">
          <span class="key-badge" id="key-w">W: Fwd</span>
          <span class="key-badge" id="key-a">A: Left</span>
          <span class="key-badge" id="key-s">S: Back</span>
          <span class="key-badge" id="key-d">D: Right</span>
          <span class="key-badge" id="key-space">Space: Stop</span>
        </div>
      </div>
      <div class="motion-grid">
        <!-- D-Pad -->
        <div class="dpad-wrapper">
          <div class="dpad">
            <button class="dpad-btn d-up" id="btn-dpad-up" onclick="runGait('walk')">▲<span>Forward</span></button>
            <button class="dpad-btn d-left" id="btn-dpad-left" onclick="runGait('left')">◀<span>Turn L</span></button>
            <button class="dpad-btn d-stop" id="btn-dpad-stop" onclick="runGait('stand')">🛑<span>Stop</span></button>
            <button class="dpad-btn d-right" id="btn-dpad-right" onclick="runGait('right')">▶<span>Turn R</span></button>
            <button class="dpad-btn d-down" id="btn-dpad-down" onclick="runGait('backward')">▼<span>Back</span></button>
          </div>
        </div>

        <!-- Action Cards Grid -->
        <div class="action-grid">
          <!-- Dedicated Start Walking Button -->
          <div class="action-card act-walk" onclick="runGait('walk')" style="grid-column: span 2; background: linear-gradient(135deg, rgba(16,185,129,0.32), rgba(6,182,212,0.25)); border: 2px solid var(--accent-emerald); box-shadow: 0 0 20px rgba(16,185,129,0.38);">
            <div class="action-title" style="color: #6ee7b7; font-size: 1.02rem; font-weight: 900;"><span>🚀 1. START DYNAMIC WALKING</span><span>▶</span></div>
            <div class="action-desc" style="color: #a7f3d0; font-weight: 700;">Initiate continuous alternating stride walking with smooth ground clearance</div>
          </div>
          <div class="action-card act-step" onclick="runGait('step')">
            <div class="action-title"><span>🐾 2. Single Stride Step</span><span>▶</span></div>
            <div class="action-desc">One complete forward stride returning to standing neutral</div>
          </div>
          <div class="action-card act-ref" onclick="runGait('ref_step')">
            <div class="action-title"><span>📐 3. Ref takeStep(1.6,20)</span><span>▶</span></div>
            <div class="action-desc">Discrete calibrated IK step sequence</div>
          </div>
          <div class="action-card act-march" onclick="runGait('march')" style="border-color: rgba(6,182,212,0.35); background: rgba(6,182,212,0.1);">
            <div class="action-title" style="color: #67e8f9;"><span>🎪 4. In-Place March</span><span>▶</span></div>
            <div class="action-desc">High-knee dynamic alternating foot lift with zero forward drift</div>
          </div>
          <div class="action-card act-crouch" onclick="runGait('crouch')" style="border-color: rgba(245,158,11,0.35); background: rgba(245,158,11,0.1);">
            <div class="action-title" style="color: #fde68a;"><span>🏋️ 5. Crouch & Squat</span><span>▶</span></div>
            <div class="action-desc">Symmetrical vertical squat with knee flexion</div>
          </div>
          <div class="action-card act-test" onclick="runGait('test')">
            <div class="action-title"><span>🧪 6. Diagnostics Sweep</span><span>▶</span></div>
            <div class="action-desc">Sequential safety sweep of all 6 servos</div>
          </div>
          <div class="action-card act-stop" onclick="runGait('stand')" style="background: linear-gradient(135deg, rgba(239,68,68,0.25), rgba(185,28,28,0.2)); border: 1px solid rgba(239,68,68,0.4); grid-column: span 2;">
            <div class="action-title" style="color: #fca5a5;"><span>🛑 7. EMERGENCY STOP & HOLD STAND</span><span>⏹</span></div>
            <div class="action-desc">Locks both legs to calibrated standing neutral [94°, 8°, 94° | 94°, 94°, 94°]</div>
          </div>
        </div>
      </div>
    </section>

    <!-- Choreographed Stunt & Gesture Routines -->
    <section class="card">
      <div class="card-header">
        <div class="card-title"><span>🎭</span><span>CHOREOGRAPHED STUNTS & SPECIAL GESTURES</span></div>
      </div>
      <div class="stunt-grid">
        <div class="stunt-card" onclick="playRoutine('bow')">
          <div class="stunt-title"><span>🙇 1. Polite Robot Bow</span><span>▶</span></div>
          <div class="stunt-desc">Graceful forward hip tilt gesture with smooth balance recovery</div>
        </div>
        <div class="stunt-card" onclick="playRoutine('kick')">
          <div class="stunt-title"><span>🥋 2. Martial Arts Kick</span><span>▶</span></div>
          <div class="stunt-desc">Weight shifts left, right leg lifts and snaps forward smoothly</div>
        </div>
        <div class="stunt-card" onclick="playRoutine('shimmy')">
          <div class="stunt-title"><span>💃 3. Dynamic Dance Shimmy</span><span>▶</span></div>
          <div class="stunt-desc">Rhythmic alternating hip-sway balance performance</div>
        </div>
        <div class="stunt-card" onclick="playRoutine('standup')">
          <div class="stunt-title"><span>🧍 4. Smooth Stand-Up Settle</span><span>▶</span></div>
          <div class="stunt-desc">Gradual leg extension from crouched posture into neutral</div>
        </div>
      </div>
    </section>

    <!-- Interactive Web Terminal & Command Console -->
    <section class="card">
      <div class="card-header">
        <div class="card-title"><span>💻</span><span>INTERACTIVE WEB TERMINAL & SYSTEM CLI</span></div>
        <button class="tab-pill" onclick="clearTerminal()">Clear Log</button>
      </div>
      <div class="terminal-container">
        <div class="term-chips">
          <span style="font-size: 0.66rem; color: var(--text-muted); margin-right: 4px;">Quick Commands:</span>
          <span class="term-chip" onclick="execTerm('walk')">walk</span>
          <span class="term-chip" onclick="execTerm('step')">step</span>
          <span class="term-chip" onclick="execTerm('march')">march</span>
          <span class="term-chip" onclick="execTerm('crouch')">crouch</span>
          <span class="term-chip" onclick="execTerm('stop')">stop</span>
          <span class="term-chip" onclick="execTerm('diag')">diag</span>
          <span class="term-chip" onclick="execTerm('status')">status</span>
          <span class="term-chip" onclick="execTerm('help')">help</span>
        </div>
        <div class="terminal-output" id="term-output">
          <div class="log-line"><span class="log-time">[BOOT]</span><span>Connected to ESP32 6-DOF Bipedal Robot Controller</span></div>
          <div class="log-line"><span class="log-time">[SYS]</span><span>Real-Time FreeRTOS Locomotion Engine active on Core 1 @ 50Hz</span></div>
        </div>
        <form class="terminal-input-bar" onsubmit="event.preventDefault(); submitTermInput();">
          <span class="term-prompt">&gt;</span>
          <input type="text" class="term-input" id="term-input" placeholder="Type command (e.g. walk, step, stop, diag, status)..." autocomplete="off">
          <button type="submit" class="term-btn">SEND</button>
        </form>
      </div>
    </section>

    <!-- Real-Time Joint Telemetry Strip -->
    <section class="card">
      <div class="card-header">
        <div class="card-title"><span>📡</span><span>REAL-TIME 6-DOF TELEMETRY</span></div>
      </div>
      <div class="telemetry-grid">
        <div class="telem-card" onclick="selectJoint(0, 'Right Hip')">
          <span class="telem-name">R.Hip (0)</span>
          <span class="telem-angle telem-r" id="track-0">94°</span>
          <div class="telem-bar-track"><div class="telem-bar-fill" id="bar-0" style="background: var(--accent-amber);"></div></div>
        </div>
        <div class="telem-card" onclick="selectJoint(1, 'Right Knee')">
          <span class="telem-name">R.Knee (1)</span>
          <span class="telem-angle telem-r" id="track-1">8°</span>
          <div class="telem-bar-track"><div class="telem-bar-fill" id="bar-1" style="background: var(--accent-amber);"></div></div>
        </div>
        <div class="telem-card" onclick="selectJoint(12, 'Right Foot')">
          <span class="telem-name">R.Foot (12)</span>
          <span class="telem-angle telem-r" id="track-12">94°</span>
          <div class="telem-bar-track"><div class="telem-bar-fill" id="bar-12" style="background: var(--accent-amber);"></div></div>
        </div>
        <div class="telem-card" onclick="selectJoint(4, 'Left Hip')">
          <span class="telem-name">L.Hip (4)</span>
          <span class="telem-angle telem-l" id="track-4">94°</span>
          <div class="telem-bar-track"><div class="telem-bar-fill" id="bar-4" style="background: var(--accent-cyan);"></div></div>
        </div>
        <div class="telem-card" onclick="selectJoint(8, 'Left Knee')">
          <span class="telem-name">L.Knee (8)</span>
          <span class="telem-angle telem-l" id="track-8">94°</span>
          <div class="telem-bar-track"><div class="telem-bar-fill" id="bar-8" style="background: var(--accent-cyan);"></div></div>
        </div>
        <div class="telem-card" onclick="selectJoint(15, 'Left Foot')">
          <span class="telem-name">L.Foot (15)</span>
          <span class="telem-angle telem-l" id="track-15">94°</span>
          <div class="telem-bar-track"><div class="telem-bar-fill" id="bar-15" style="background: var(--accent-cyan);"></div></div>
        </div>
      </div>
    </section>

    <!-- Real-Time Ground Reaction Force & Weight Distribution HUD Card -->
    <section class="card">
      <div class="card-header">
        <div class="card-title">
          <span>⚖️</span>
          <span>GROUND REACTION FORCE & WEIGHT DISTRIBUTION (WALKING)</span>
        </div>
        <div class="hud-badge" style="color: var(--accent-emerald);">
          <span id="hud-contact-state">DUAL STANCE</span>
        </div>
      </div>
      <div class="weight-grid">
        <!-- Left Foot Ground Weight -->
        <div class="weight-card weight-card-l">
          <div class="weight-title-row">
            <span class="weight-title" style="color: var(--accent-cyan);">Left Foot Ground Load</span>
            <span class="hud-badge" id="weight-pct-l" style="color: var(--accent-cyan);">50%</span>
          </div>
          <div style="display: flex; justify-content: space-between; align-items: baseline;">
            <span class="weight-val" id="weight-val-l" style="color: var(--accent-cyan);">250g</span>
            <span style="font-size: 0.72rem; color: var(--text-muted);">Max: 500g</span>
          </div>
          <div class="weight-bar-bg">
            <div class="weight-bar-fill" id="weight-bar-l" style="width: 50%; background: linear-gradient(90deg, #0284c7, #06b6d4);"></div>
          </div>
        </div>

        <!-- Right Foot Ground Weight -->
        <div class="weight-card weight-card-r">
          <div class="weight-title-row">
            <span class="weight-title" style="color: var(--accent-amber);">Right Foot Ground Load</span>
            <span class="hud-badge" id="weight-pct-r" style="color: var(--accent-amber);">50%</span>
          </div>
          <div style="display: flex; justify-content: space-between; align-items: baseline;">
            <span class="weight-val" id="weight-val-r" style="color: var(--accent-amber);">250g</span>
            <span style="font-size: 0.72rem; color: var(--text-muted);">Max: 500g</span>
          </div>
          <div class="weight-bar-bg">
            <div class="weight-bar-fill" id="weight-bar-r" style="width: 50%; background: linear-gradient(90deg, #d97706, #f59e0b);"></div>
          </div>
        </div>
      </div>

      <div class="grf-summary-bar">
        <div style="display: flex; align-items: center; gap: 8px;">
          <span style="font-size: 0.75rem; color: var(--text-muted);">TOTAL DYNAMIC FORCE:</span>
          <span style="font-size: 1.1rem; font-weight: 900; color: #fff; font-family: ui-monospace, monospace;" id="hud-force-n">4.91 N</span>
        </div>
        <div style="display: flex; align-items: center; gap: 8px;">
          <span style="font-size: 0.75rem; color: var(--text-muted);">TOTAL ROBOT MASS:</span>
          <span style="font-size: 0.95rem; font-weight: 800; color: var(--accent-sky); font-family: ui-monospace, monospace;">500 g</span>
        </div>
        <div style="display: flex; align-items: center; gap: 8px;">
          <span style="font-size: 0.75rem; color: var(--text-muted);">SUPPORT STABILITY:</span>
          <span style="font-size: 0.75rem; font-weight: 800; color: var(--accent-emerald);" id="hud-zmp-status">STABLE (ZMP CENTERED)</span>
        </div>
      </div>
    </section>

    <!-- Right Leg Joint Controls -->
    <div class="leg-section-title leg-r-header">
      <span>RIGHT LEG JOINTS (SAGITTAL PLANE)</span>
      <span style="font-size: 0.72rem; font-weight: 800;">CH 0: Hip (94°) | CH 1: Knee (8°) | CH 12: Foot (94°)</span>
    </div>
    <div class="servo-grid">
      <!-- CH 0 -->
      <div class="servo-card servo-card-r" id="card-0">
        <div class="servo-header">
          <span class="servo-name" style="color: var(--accent-amber);">Right Hip (CH 0)</span>
          <div class="servo-angle-box">
            <span class="servo-angle-text" id="angle-0">94°</span>
            <span class="servo-home-tag">(Home: 94°)</span>
          </div>
        </div>
        <input type="range" class="slider-styled" id="slider-0" min="0" max="180" value="94" oninput="setAngle(0, this.value)" onchange="setAngle(0, this.value)">
        <div class="step-btn-group">
          <button class="step-btn" onclick="stepAngle(0, -10)">-10</button>
          <button class="step-btn" onclick="stepAngle(0, -5)">-5</button>
          <button class="step-btn" onclick="stepAngle(0, -1)">-1</button>
          <button class="step-btn btn-reset-single" onclick="resetSingleJoint(0, 94)">94°</button>
          <button class="step-btn" onclick="stepAngle(0, 1)">+1</button>
          <button class="step-btn" onclick="stepAngle(0, 5)">+5</button>
          <button class="step-btn" onclick="stepAngle(0, 10)">+10</button>
        </div>
      </div>

      <!-- CH 1 -->
      <div class="servo-card servo-card-r" id="card-1">
        <div class="servo-header">
          <span class="servo-name" style="color: var(--accent-amber);">Right Knee (CH 1)</span>
          <div class="servo-angle-box">
            <span class="servo-angle-text" id="angle-1">8°</span>
            <span class="servo-home-tag">(Home: 8°)</span>
          </div>
        </div>
        <input type="range" class="slider-styled" id="slider-1" min="0" max="180" value="8" oninput="setAngle(1, this.value)" onchange="setAngle(1, this.value)">
        <div class="step-btn-group">
          <button class="step-btn" onclick="stepAngle(1, -10)">-10</button>
          <button class="step-btn" onclick="stepAngle(1, -5)">-5</button>
          <button class="step-btn" onclick="stepAngle(1, -1)">-1</button>
          <button class="step-btn btn-reset-single" onclick="resetSingleJoint(1, 8)">8°</button>
          <button class="step-btn" onclick="stepAngle(1, 1)">+1</button>
          <button class="step-btn" onclick="stepAngle(1, 5)">+5</button>
          <button class="step-btn" onclick="stepAngle(1, 10)">+10</button>
        </div>
      </div>

      <!-- CH 12 -->
      <div class="servo-card servo-card-r" id="card-12">
        <div class="servo-header">
          <span class="servo-name" style="color: var(--accent-amber);">Right Foot (CH 12)</span>
          <div class="servo-angle-box">
            <span class="servo-angle-text" id="angle-12">94°</span>
            <span class="servo-home-tag">(Home: 94°)</span>
          </div>
        </div>
        <input type="range" class="slider-styled" id="slider-12" min="0" max="180" value="94" oninput="setAngle(12, this.value)" onchange="setAngle(12, this.value)">
        <div class="step-btn-group">
          <button class="step-btn" onclick="stepAngle(12, -10)">-10</button>
          <button class="step-btn" onclick="stepAngle(12, -5)">-5</button>
          <button class="step-btn" onclick="stepAngle(12, -1)">-1</button>
          <button class="step-btn btn-reset-single" onclick="resetSingleJoint(12, 94)">94°</button>
          <button class="step-btn" onclick="stepAngle(12, 1)">+1</button>
          <button class="step-btn" onclick="stepAngle(12, 5)">+5</button>
          <button class="step-btn" onclick="stepAngle(12, 10)">+10</button>
        </div>
      </div>
    </div>

    <!-- Left Leg Joint Controls -->
    <div class="leg-section-title leg-l-header">
      <span>LEFT LEG JOINTS (SAGITTAL PLANE - MIRRORED)</span>
      <span style="font-size: 0.72rem; font-weight: 800;">CH 4: Hip (94°) | CH 8: Knee (94°) | CH 15: Foot (94°)</span>
    </div>
    <div class="servo-grid">
      <!-- CH 4 -->
      <div class="servo-card" id="card-4">
        <div class="servo-header">
          <span class="servo-name" style="color: var(--accent-cyan);">Left Hip (CH 4)</span>
          <div class="servo-angle-box">
            <span class="servo-angle-text" id="angle-4">94°</span>
            <span class="servo-home-tag">(Home: 94°)</span>
          </div>
        </div>
        <input type="range" class="slider-styled" id="slider-4" min="0" max="180" value="94" oninput="setAngle(4, this.value)" onchange="setAngle(4, this.value)">
        <div class="step-btn-group">
          <button class="step-btn" onclick="stepAngle(4, -10)">-10</button>
          <button class="step-btn" onclick="stepAngle(4, -5)">-5</button>
          <button class="step-btn" onclick="stepAngle(4, -1)">-1</button>
          <button class="step-btn btn-reset-single" onclick="resetSingleJoint(4, 94)">94°</button>
          <button class="step-btn" onclick="stepAngle(4, 1)">+1</button>
          <button class="step-btn" onclick="stepAngle(4, 5)">+5</button>
          <button class="step-btn" onclick="stepAngle(4, 10)">+10</button>
        </div>
      </div>

      <!-- CH 8 -->
      <div class="servo-card" id="card-8">
        <div class="servo-header">
          <span class="servo-name" style="color: var(--accent-cyan);">Left Knee (CH 8)</span>
          <div class="servo-angle-box">
            <span class="servo-angle-text" id="angle-8">94°</span>
            <span class="servo-home-tag">(Home: 94°)</span>
          </div>
        </div>
        <input type="range" class="slider-styled" id="slider-8" min="0" max="180" value="94" oninput="setAngle(8, this.value)" onchange="setAngle(8, this.value)">
        <div class="step-btn-group">
          <button class="step-btn" onclick="stepAngle(8, -10)">-10</button>
          <button class="step-btn" onclick="stepAngle(8, -5)">-5</button>
          <button class="step-btn" onclick="stepAngle(8, -1)">-1</button>
          <button class="step-btn btn-reset-single" onclick="resetSingleJoint(8, 94)">94°</button>
          <button class="step-btn" onclick="stepAngle(8, 1)">+1</button>
          <button class="step-btn" onclick="stepAngle(8, 5)">+5</button>
          <button class="step-btn" onclick="stepAngle(8, 10)">+10</button>
        </div>
      </div>

      <!-- CH 15 -->
      <div class="servo-card" id="card-15">
        <div class="servo-header">
          <span class="servo-name" style="color: var(--accent-cyan);">Left Foot (CH 15)</span>
          <div class="servo-angle-box">
            <span class="servo-angle-text" id="angle-15">94°</span>
            <span class="servo-home-tag">(Home: 94°)</span>
          </div>
        </div>
        <input type="range" class="slider-styled" id="slider-15" min="0" max="180" value="94" oninput="setAngle(15, this.value)" onchange="setAngle(15, this.value)">
        <div class="step-btn-group">
          <button class="step-btn" onclick="stepAngle(15, -10)">-10</button>
          <button class="step-btn" onclick="stepAngle(15, -5)">-5</button>
          <button class="step-btn" onclick="stepAngle(15, -1)">-1</button>
          <button class="step-btn btn-reset-single" onclick="resetSingleJoint(15, 94)">94°</button>
          <button class="step-btn" onclick="stepAngle(15, 1)">+1</button>
          <button class="step-btn" onclick="stepAngle(15, 5)">+5</button>
          <button class="step-btn" onclick="stepAngle(15, 10)">+10</button>
        </div>
      </div>
    </div>

    <!-- Dynamic Gait Physics Tuner -->
    <section class="card">
      <div class="card-header">
        <div class="card-title"><span>📐</span><span>DYNAMIC GAIT PHYSICS & INVERSE KINEMATICS</span></div>
      </div>
      <div class="tuner-grid">
        <div class="tuner-card">
          <div class="tuner-header"><span class="tuner-label">Cadence (Speed)</span><span class="tuner-val" id="val-freq">0.55 Hz</span></div>
          <input type="range" class="slider-styled" id="slider-freq" min="3" max="14" value="5.5" step="0.5" oninput="setGaitParam('freq', this.value / 10.0)">
        </div>
        <div class="tuner-card">
          <div class="tuner-header"><span class="tuner-label">Stride Length</span><span class="tuner-val" id="val-stride">0.80 cm</span></div>
          <input type="range" class="slider-styled" id="slider-stride" min="0.4" max="2.5" value="0.80" step="0.05" oninput="setGaitParam('stride', this.value)">
        </div>
        <div class="tuner-card">
          <div class="tuner-header"><span class="tuner-label">Swing Clearance</span><span class="tuner-val" id="val-knee">0.40 cm</span></div>
          <input type="range" class="slider-styled" id="slider-knee" min="0.2" max="1.5" value="0.40" step="0.05" oninput="setGaitParam('knee', this.value)">
        </div>
        <div class="tuner-card">
          <div class="tuner-header"><span class="tuner-label">Stance Height</span><span class="tuner-val" id="val-height">10.0 cm</span></div>
          <input type="range" class="slider-styled" id="slider-height" min="8.5" max="10.7" value="10.0" step="0.1" oninput="setGaitParam('height', this.value)">
        </div>
      </div>
    </section>

    <!-- System Actions & Non-Volatile Storage -->
    <section class="card">
      <div class="card-header">
        <div class="card-title"><span>💾</span><span>NON-VOLATILE STORAGE & SYSTEM HARDWARE</span></div>
      </div>
      <div class="sys-actions">
        <button class="btn-sys btn-save" onclick="saveOffsets()"><span>💾</span><span>Save Offsets To EEPROM</span></button>
        <button class="btn-sys btn-restore" onclick="resetOffsets()"><span>🔄</span><span>Restore Factory Baselines</span></button>
        <button class="btn-sys btn-export" onclick="exportProfile()"><span>📤</span><span>Export JSON Profile</span></button>
        <button class="btn-sys btn-scan" onclick="scanI2C()"><span>🔍</span><span>Scan I2C Bus</span></button>
      </div>
    </section>
  </div>

  <!-- Non-intrusive Toast Notification -->
  <div id="toast"><span>✓</span><span id="toast-text">Operation Successful</span></div>

  <script>
    let sendTimers = {};
    let selectedJointChannel = 0;
    let selectedJointName = 'Right Hip';
    const DEFAULT_ANGLES = { 0: 94, 1: 8, 12: 94, 4: 94, 8: 94, 15: 94 };
    let liveAngles = { ...DEFAULT_ANGLES };

    // Scope variables
    let scopeRunning = true;
    let scopeHistory = [];
    const MAX_SCOPE_POINTS = 120;

    function showToast(msg, duration = 2500) {
      const t = document.getElementById('toast');
      const txt = document.getElementById('toast-text');
      if (t && txt) {
        txt.innerText = msg;
        t.classList.add('show');
        clearTimeout(t.timer);
        t.timer = setTimeout(() => { t.classList.remove('show'); }, duration);
      }
    }

    function sendCmd(url, msg = null) {
      fetch(url)
        .then(r => r.json())
        .then(d => {
          if (msg) showToast(msg);
        })
        .catch(() => {
          if (msg) showToast(msg);
        });
    }

    function switchSchematic(tab) {
      const kSvg = document.getElementById('svg-kinematic');
      const sSvg = document.getElementById('svg-side');
      const cSvg = document.getElementById('svg-circuit');
      const kBtn = document.getElementById('tab-btn-kinematic');
      const sBtn = document.getElementById('tab-btn-side');
      const cBtn = document.getElementById('tab-btn-circuit');

      if (kSvg) kSvg.style.display = (tab === 'kinematic') ? 'block' : 'none';
      if (sSvg) sSvg.style.display = (tab === 'side') ? 'block' : 'none';
      if (cSvg) cSvg.style.display = (tab === 'circuit') ? 'block' : 'none';

      if (kBtn) kBtn.classList.toggle('active', tab === 'kinematic');
      if (sBtn) sBtn.classList.toggle('active', tab === 'side');
      if (cBtn) cBtn.classList.toggle('active', tab === 'circuit');
    }

    function selectJoint(ch, name) {
      selectedJointChannel = ch;
      selectedJointName = name;
      document.getElementById('selected-joint-badge').innerText = name + ' (CH ' + ch + ')';

      const slider = document.getElementById('slider-' + ch);
      if (slider) {
        document.getElementById('selected-joint-angle').innerText = slider.value + '°';
      }

      [0, 1, 12, 4, 8, 15].forEach(c => {
        const node = document.getElementById('node-' + c);
        if (node) {
          if (c === ch) node.classList.add('selected');
          else node.classList.remove('selected');
        }
        const card = document.getElementById('card-' + c);
        if (card) {
          if (c === ch) card.classList.add('active-target');
          else card.classList.remove('active-target');
        }
      });
    }

    function stepSelected(delta) {
      stepAngle(selectedJointChannel, delta);
    }

    function homeSelected() {
      const home = DEFAULT_ANGLES[selectedJointChannel] || 94;
      setAngle(selectedJointChannel, home);
      showToast('Reset CH ' + selectedJointChannel + ' to ' + home + '°');
    }

    function resetSingleJoint(ch, homeAngle) {
      setAngle(ch, homeAngle);
      showToast('Reset CH ' + ch + ' to ' + homeAngle + '°');
    }

    function setAngle(ch, val) {
      val = parseInt(val);
      liveAngles[ch] = val;
      updateUI(ch, val);
      animateRig(liveAngles);
      if (ch === selectedJointChannel) {
        document.getElementById('selected-joint-angle').innerText = val + '°';
      }
      clearTimeout(sendTimers[ch]);
      sendTimers[ch] = setTimeout(() => {
        sendCmd('/set_servo?ch=' + ch + '&val=' + val);
      }, 20);
    }

    function updateUI(ch, val) {
      const el = document.getElementById('angle-' + ch);
      const sl = document.getElementById('slider-' + ch);
      const tr = document.getElementById('track-' + ch);
      const br = document.getElementById('bar-' + ch);
      const svgA = document.getElementById('svg-angle-' + ch);

      if (el) el.innerText = val + '°';
      if (sl) sl.value = val;
      if (tr) tr.innerText = val + '°';
      if (br) br.style.width = ((val / 180) * 100) + '%';
      if (svgA) svgA.textContent = val + '°';

      if (ch === selectedJointChannel) {
        const sja = document.getElementById('selected-joint-angle');
        if (sja) sja.innerText = val + '°';
      }
    }

    function stepAngle(ch, delta) {
      const slider = document.getElementById('slider-' + ch);
      if (slider) {
        let val = parseInt(slider.value) + delta;
        if (val < 0) val = 0;
        if (val > 180) val = 180;
        setAngle(ch, val);
      }
    }

    function runGait(gait) {
      const msgs = {
        'start': '🟢 Robot Initialized in Standard Stand',
        'walk': '🚀 Continuous Dynamic Walk Started',
        'step': '🐾 Single Full Stride Executing',
        'stand': '🛑 Robot Parked at Constant Neutral',
        'ref_step': '📐 Reference takeStep(1.6, 20) Executing',
        'ref_init': '⚙️ Reference initialize() Executing',
        'test': '🧪 Running Joint Diagnostics Sweep',
        'left': '◀ Turning Left',
        'right': '▶ Turning Right',
        'backward': '▼ Walking Backward',
        'march': '🎪 In-Place Marching Active',
        'crouch': '🏋️ Crouch & Squat Active'
      };
      sendCmd('/run_gait?gait=' + gait, msgs[gait] || ('Gait: ' + gait));
      addTermLog('[CMD] Executed gait: ' + gait);
    }

    function setGaitParam(param, val) {
      val = parseFloat(val);
      if (param === 'freq') {
        const el = document.getElementById('val-freq');
        if (el) el.innerText = val.toFixed(2) + ' Hz';
      } else if (param === 'stride') {
        const el = document.getElementById('val-stride');
        if (el) el.innerText = val.toFixed(2) + ' cm';
      } else if (param === 'knee') {
        const el = document.getElementById('val-knee');
        if (el) el.innerText = val.toFixed(2) + ' cm';
      } else if (param === 'height') {
        const el = document.getElementById('val-height');
        if (el) el.innerText = val.toFixed(1) + ' cm';
      }
      sendCmd('/set_gait_param?param=' + param + '&val=' + val);
    }

    function saveOffsets() {
      sendCmd('/save_offsets', '💾 All 6 Joint Angles Saved Permanently to Flash!');
      addTermLog('[NVS] Saved active joint positions as permanent home baselines');
    }

    function resetOffsets() {
      sendCmd('/reset_offsets', '🔄 Restored Factory Baselines: Right [94°, 8°, 94°] | Left [94°, 94°, 94°]');
      addTermLog('[NVS] Factory defaults restored');
      setTimeout(pollTelemetry, 300);
    }

    function exportProfile() {
      const profile = {
        name: "ESP32 Biped Robot Calibration",
        timestamp: new Date().toISOString(),
        angles: {
          ch0_r_hip: parseInt(document.getElementById('slider-0')?.value || 94),
          ch1_r_knee: parseInt(document.getElementById('slider-1')?.value || 8),
          ch12_r_foot: parseInt(document.getElementById('slider-12')?.value || 94),
          ch4_l_hip: parseInt(document.getElementById('slider-4')?.value || 94),
          ch8_l_knee: parseInt(document.getElementById('slider-8')?.value || 94),
          ch15_l_foot: parseInt(document.getElementById('slider-15')?.value || 94)
        }
      };
      const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(profile, null, 2));
      const dlAnchor = document.createElement('a');
      dlAnchor.setAttribute("href", dataStr);
      dlAnchor.setAttribute("download", "biped_calibration_profile.json");
      document.body.appendChild(dlAnchor);
      dlAnchor.click();
      dlAnchor.remove();
      showToast('📤 Calibration Profile Exported!');
    }

    function scanI2C() {
      fetch('/scan_i2c')
        .then(r => r.json())
        .then(d => {
          if (d.devices && d.devices.length > 0) {
            showToast('✓ I2C Bus Devices Found: ' + d.devices.join(', '));
            addTermLog('[I2C] Found responsive devices: ' + d.devices.join(', '));
          } else {
            showToast('⚠️ No I2C Devices Found on SDA 21 / SCL 22');
            addTermLog('[I2C] No devices detected');
          }
        }).catch(() => { showToast('I2C Scan Completed'); });
    }

    // =========================================================================
    // 2D Robot Movement Kinematics Engine (Live Movement Visualizer)
    // =========================================================================
    function setLine(id, x1, y1, x2, y2) {
      const el = document.getElementById(id);
      if (el) {
        el.setAttribute('x1', x1); el.setAttribute('y1', y1);
        el.setAttribute('x2', x2); el.setAttribute('y2', y2);
      }
    }
    function setCircle(id, cx, cy) {
      const el = document.getElementById(id);
      if (el) { el.setAttribute('cx', cx); el.setAttribute('cy', cy); }
    }
    function setRect(id, x, y) {
      const el = document.getElementById(id);
      if (el) { el.setAttribute('x', x); el.setAttribute('y', y); }
    }

    function animateRig(angles) {
      const rHipA  = angles['0']  !== undefined ? angles['0']  : 94;
      const rKneeA = angles['1']  !== undefined ? angles['1']  : 8;
      const rFootA = angles['12'] !== undefined ? angles['12'] : 94;
      const lHipA  = angles['4']  !== undefined ? angles['4']  : 94;
      const lKneeA = angles['8']  !== undefined ? angles['8']  : 94;
      const lFootA = angles['15'] !== undefined ? angles['15'] : 94;

      // 1. FRONT VIEW KINEMATIC RIG
      // Right Leg Front View
      const dHipR = (rHipA - 94.0) * (Math.PI / 180);
      const dKneeR = (rKneeA - 8.0) * (Math.PI / 180);
      const rKneeX = 140 + 95 * Math.sin(dHipR * 0.45);
      const rKneeY = 100 + 95 * Math.cos(dHipR * 0.45);
      const rAnkleX = rKneeX + 105 * Math.sin((dHipR - dKneeR) * 0.45);
      const rAnkleY = rKneeY + 105 * Math.cos((dHipR - dKneeR) * 0.45);

      setLine('link-thigh-r', 140, 100, rKneeX, rKneeY);
      setLine('link-shin-r', rKneeX, rKneeY, rAnkleX, rAnkleY);
      const node1 = document.getElementById('node-1');
      if (node1) node1.setAttribute('transform', `translate(${rKneeX - 140}, ${rKneeY - 200})`);
      const node12 = document.getElementById('node-12');
      if (node12) node12.setAttribute('transform', `translate(${rAnkleX - 140}, ${rAnkleY - 300})`);
      const footPlateR = document.getElementById('plate-foot-r');
      if (footPlateR) { 
        footPlateR.setAttribute('x', rAnkleX - 35); 
        footPlateR.setAttribute('y', rAnkleY + 14);
        footPlateR.setAttribute('transform', `rotate(${(rFootA - 94.0) * 0.8}, ${rAnkleX}, ${rAnkleY + 14})`);
      }
      const footTextR = document.getElementById('plate-text-r');
      if (footTextR) { 
        footTextR.setAttribute('x', rAnkleX); 
        footTextR.setAttribute('y', rAnkleY + 25);
        footTextR.setAttribute('transform', `rotate(${(rFootA - 94.0) * 0.8}, ${rAnkleX}, ${rAnkleY + 14})`);
      }

      // Left Leg Front View
      const dHipL = -(lHipA - 94.0) * (Math.PI / 180);
      const dKneeL = (94.0 - lKneeA) * (Math.PI / 180);
      const lKneeX = 400 + 95 * Math.sin(dHipL * 0.45);
      const lKneeY = 100 + 95 * Math.cos(dHipL * 0.45);
      const lAnkleX = lKneeX + 105 * Math.sin((dHipL - dKneeL) * 0.45);
      const lAnkleY = lKneeY + 105 * Math.cos((dHipL - dKneeL) * 0.45);

      setLine('link-thigh-l', 400, 100, lKneeX, lKneeY);
      setLine('link-shin-l', lKneeX, lKneeY, lAnkleX, lAnkleY);
      const node8 = document.getElementById('node-8');
      if (node8) node8.setAttribute('transform', `translate(${lKneeX - 400}, ${lKneeY - 200})`);
      const node15 = document.getElementById('node-15');
      if (node15) node15.setAttribute('transform', `translate(${lAnkleX - 400}, ${lAnkleY - 300})`);
      const footPlateL = document.getElementById('plate-foot-l');
      if (footPlateL) { 
        footPlateL.setAttribute('x', lAnkleX - 35); 
        footPlateL.setAttribute('y', lAnkleY + 14);
        footPlateL.setAttribute('transform', `rotate(${-(lFootA - 94.0) * 0.8}, ${lAnkleX}, ${lAnkleY + 14})`);
      }
      const footTextL = document.getElementById('plate-text-l');
      if (footTextL) { 
        footTextL.setAttribute('x', lAnkleX); 
        footTextL.setAttribute('y', lAnkleY + 25);
        footTextL.setAttribute('transform', `rotate(${-(lFootA - 94.0) * 0.8}, ${lAnkleX}, ${lAnkleY + 14})`);
      }

      // 2. SIDE SAGITTAL WALKING RIG (Walking Profile with Stride, Knee Lift & Dynamic Ankle Flexion)
      const hipX = 270, hipY = 80;
      const L1 = 110, L2 = 120; // visual link lengths

      // Right Leg Side Profile:
      const radHipR = -(rHipA - 94.0) * (Math.PI / 180) * 1.8;
      const radKneeR = (rKneeA - 8.0) * (Math.PI / 180) * 2.2;
      const skR_x = hipX + L1 * Math.sin(radHipR);
      const skR_y = hipY + L1 * Math.cos(radHipR);
      const saR_x = skR_x + L2 * Math.sin(radHipR - radKneeR);
      const saR_y = skR_y + L2 * Math.cos(radHipR - radKneeR);

      setLine('side-thigh-r', hipX, hipY, skR_x, skR_y);
      setCircle('side-knee-r', skR_x, skR_y);
      setLine('side-shin-r', skR_x, skR_y, saR_x, saR_y);
      setCircle('side-ankle-r', saR_x, saR_y);
      
      const footDegR = (rFootA - 94.0) * 1.5;
      const elFootR = document.getElementById('side-foot-r');
      if (elFootR) {
        elFootR.setAttribute('x', saR_x - 32);
        elFootR.setAttribute('y', saR_y + 4);
        elFootR.setAttribute('transform', `rotate(${footDegR}, ${saR_x}, ${saR_y})`);
      }
      const txtFootR = document.getElementById('side-foot-text-r');
      if (txtFootR) { 
        txtFootR.setAttribute('x', saR_x); 
        txtFootR.setAttribute('y', saR_y + 15);
        txtFootR.setAttribute('transform', `rotate(${footDegR}, ${saR_x}, ${saR_y})`);
      }

      // Left Leg Side Profile:
      const radHipL = (lHipA - 94.0) * (Math.PI / 180) * 1.8;
      const radKneeL = (94.0 - lKneeA) * (Math.PI / 180) * 2.2;
      const skL_x = hipX + L1 * Math.sin(radHipL);
      const skL_y = hipY + L1 * Math.cos(radHipL);
      const saL_x = skL_x + L2 * Math.sin(radHipL - radKneeL);
      const saL_y = skL_y + L2 * Math.cos(radHipL - radKneeL);

      setLine('side-thigh-l', hipX, hipY, skL_x, skL_y);
      setCircle('side-knee-l', skL_x, skL_y);
      setLine('side-shin-l', skL_x, skL_y, saL_x, saL_y);
      setCircle('side-ankle-l', saL_x, saL_y);
      
      const footDegL = -(lFootA - 94.0) * 1.5;
      const elFootL = document.getElementById('side-foot-l');
      if (elFootL) {
        elFootL.setAttribute('x', saL_x - 32);
        elFootL.setAttribute('y', saL_y + 4);
        elFootL.setAttribute('transform', `rotate(${footDegL}, ${saL_x}, ${saL_y})`);
      }
      const txtFootL = document.getElementById('side-foot-text-l');
      if (txtFootL) { 
        txtFootL.setAttribute('x', saL_x); 
        txtFootL.setAttribute('y', saL_y + 15);
        txtFootL.setAttribute('transform', `rotate(${footDegL}, ${saL_x}, ${saL_y})`);
      }
    }

    // =========================================================================
    // Choreographed Stunts & Animated Gestures
    // =========================================================================
    function playRoutine(name) {
      if (name === 'bow') {
        showToast('🙇 Executing Robot Bow...');
        addTermLog('[ROUTINE] Robot Bow executing');
        setAngle(0, 102); setAngle(4, 86);
        setTimeout(() => {
          setAngle(0, 94); setAngle(4, 94);
          showToast('✓ Bow Complete');
        }, 1400);
      } else if (name === 'kick') {
        showToast('🥋 Executing Martial Arts Kick...');
        addTermLog('[ROUTINE] Martial Arts Kick executing');
        setAngle(0, 98); setAngle(1, 15);
        setTimeout(() => {
          setAngle(0, 94); setAngle(1, 8);
          showToast('✓ Kick Complete');
        }, 1200);
      } else if (name === 'shimmy') {
        showToast('💃 Executing Dance Shimmy...');
        addTermLog('[ROUTINE] Dance Shimmy executing');
        let count = 0;
        let intv = setInterval(() => {
          if (count % 2 === 0) {
            setAngle(0, 98); setAngle(4, 98);
          } else {
            setAngle(0, 90); setAngle(4, 90);
          }
          count++;
          if (count > 6) {
            clearInterval(intv);
            setAngle(0, 94); setAngle(4, 94);
            showToast('✓ Shimmy Complete');
          }
        }, 350);
      } else if (name === 'standup') {
        showToast('🧍 Initializing Stand-Up...');
        runGait('init');
      }
    }

    // =========================================================================
    // Real-Time Oscilloscope Visualizer
    // =========================================================================
    const scopeCanvas = document.getElementById('scope-canvas');
    const ctx = scopeCanvas ? scopeCanvas.getContext('2d') : null;

    function resizeScope() {
      if (!scopeCanvas) return;
      scopeCanvas.width = scopeCanvas.parentElement.clientWidth;
      scopeCanvas.height = scopeCanvas.parentElement.clientHeight;
    }
    window.addEventListener('resize', resizeScope);
    setTimeout(resizeScope, 100);

    function toggleScope() {
      scopeRunning = !scopeRunning;
      const btn = document.getElementById('btn-scope-pause');
      if (btn) btn.innerText = scopeRunning ? 'Pause' : 'Resume';
    }

    function clearScope() {
      scopeHistory = [];
      drawScope();
    }

    function pushScopePoint(rh, lh, rk, lk) {
      if (!scopeRunning) return;
      scopeHistory.push({ rh, lh, rk, lk });
      if (scopeHistory.length > MAX_SCOPE_POINTS) {
        scopeHistory.shift();
      }
      drawScope();
    }

    function drawScope() {
      if (!ctx || !scopeCanvas) return;
      const w = scopeCanvas.width;
      const h = scopeCanvas.height;

      ctx.fillStyle = '#050811';
      ctx.fillRect(0, 0, w, h);

      // Grid lines
      ctx.strokeStyle = 'rgba(255,255,255,0.06)';
      ctx.lineWidth = 1;
      for (let y = 30; y < h; y += 35) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
      }

      if (scopeHistory.length < 2) return;

      const step = w / (MAX_SCOPE_POINTS - 1);

      function plotSeries(key, color, minA, maxA) {
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.beginPath();
        for (let i = 0; i < scopeHistory.length; i++) {
          const val = scopeHistory[i][key];
          const norm = (val - minA) / (maxA - minA);
          const py = h - 15 - Math.max(0, Math.min(1, norm)) * (h - 30);
          const px = i * step;
          if (i === 0) ctx.moveTo(px, py);
          else ctx.lineTo(px, py);
        }
        ctx.stroke();
      }

      plotSeries('rh', '#f59e0b', 80, 110);
      plotSeries('lh', '#06b6d4', 80, 110);
      plotSeries('rk', '#eab308', 5, 20);
      plotSeries('lk', '#3b82f6', 80, 100);
    }

    // =========================================================================
    // Interactive Web Terminal CLI
    // =========================================================================
    function addTermLog(text) {
      const out = document.getElementById('term-output');
      if (!out) return;
      const line = document.createElement('div');
      line.className = 'log-line';
      const timeStr = new Date().toTimeString().split(' ')[0];
      line.innerHTML = '<span class="log-time">[' + timeStr + ']</span><span>' + text + '</span>';
      out.appendChild(line);
      out.scrollTop = out.scrollHeight;
    }

    function clearTerminal() {
      const out = document.getElementById('term-output');
      if (out) out.innerHTML = '';
      addTermLog('[SYS] Terminal buffer cleared');
    }

    function execTerm(cmd) {
      addTermLog('&gt; ' + cmd);
      fetch('/cmd?c=' + encodeURIComponent(cmd))
        .then(r => r.json())
        .then(d => {
          if (d.output) addTermLog(d.output);
        })
        .catch(() => {
          addTermLog('[ERR] Failed to execute command');
        });
    }

    function submitTermInput() {
      const input = document.getElementById('term-input');
      if (!input || !input.value.trim()) return;
      execTerm(input.value.trim());
      input.value = '';
    }

    // =========================================================================
    // Keyboard WASD Hotkeys & Gamepad Integration
    // =========================================================================
    const activeKeys = {};
    window.addEventListener('keydown', (e) => {
      if (document.activeElement && document.activeElement.id === 'term-input') return;

      const key = e.key.toLowerCase();
      if (activeKeys[key]) return;
      activeKeys[key] = true;

      if (key === 'w' || e.key === 'ArrowUp') {
        highlightKey('key-w', true); runGait('walk');
      } else if (key === 's' || e.key === 'ArrowDown') {
        highlightKey('key-s', true); runGait('backward');
      } else if (key === 'a' || e.key === 'ArrowLeft') {
        highlightKey('key-a', true); runGait('left');
      } else if (key === 'd' || e.key === 'ArrowRight') {
        highlightKey('key-d', true); runGait('right');
      } else if (key === ' ') {
        e.preventDefault();
        highlightKey('key-space', true); runGait('stand');
      } else if (key === 'm') {
        runGait('march');
      } else if (key === 'c') {
        runGait('crouch');
      } else if (key === 't') {
        runGait('test');
      } else if (key >= '1' && key <= '6') {
        const chMap = ['0', '1', '12', '4', '8', '15'];
        const names = ['Right Hip', 'Right Knee', 'Right Foot', 'Left Hip', 'Left Knee', 'Left Foot'];
        const idx = parseInt(key) - 1;
        selectJoint(parseInt(chMap[idx]), names[idx]);
      } else if (key === 'q') {
        stepSelected(-1);
      } else if (key === 'e') {
        stepSelected(+1);
      }
    });

    window.addEventListener('keyup', (e) => {
      const key = e.key.toLowerCase();
      activeKeys[key] = false;
      if (key === 'w' || e.key === 'ArrowUp') highlightKey('key-w', false);
      if (key === 's' || e.key === 'ArrowDown') highlightKey('key-s', false);
      if (key === 'a' || e.key === 'ArrowLeft') highlightKey('key-a', false);
      if (key === 'd' || e.key === 'ArrowRight') highlightKey('key-d', false);
      if (key === ' ') highlightKey('key-space', false);
    });

    function highlightKey(id, pressed) {
      const el = document.getElementById(id);
      if (el) {
        if (pressed) el.classList.add('pressed');
        else el.classList.remove('pressed');
      }
    }

    window.addEventListener("gamepadconnected", (e) => {
      const gp = document.getElementById('stat-gamepad');
      if (gp) { gp.innerText = 'CONNECTED (' + e.gamepad.id.substring(0, 12) + ')'; gp.style.color = '#10b981'; }
      showToast('🎮 Gamepad Connected: ' + e.gamepad.id);
    });

    window.addEventListener("gamepaddisconnected", () => {
      const gp = document.getElementById('stat-gamepad');
      if (gp) { gp.innerText = 'DISCONNECTED'; gp.style.color = 'var(--text-muted)'; }
    });

    // =========================================================================
    // Real-Time Telemetry Polling (350ms interval)
    // =========================================================================
    function pollTelemetry() {
      const t0 = performance.now();
      fetch('/get_angles')
        .then(r => r.json())
        .then(d => {
          const rtt = Math.round(performance.now() - t0);
          const elPing = document.getElementById('hud-ping');
          if (elPing) elPing.innerText = rtt;

          if (d.angles) {
            for (const ch in d.angles) {
              const val = d.angles[ch];
              liveAngles[ch] = val;
              updateUI(ch, val);
            }
            // Stream to real-time oscilloscope
            pushScopePoint(
              d.angles['0'] || 94,
              d.angles['4'] || 94,
              d.angles['1'] || 8,
              d.angles['8'] || 94
            );
            // Animate 2D Robot Movement Rig
            animateRig(d.angles);
          }
          if (d.active_gait) {
            const ag = document.getElementById('svg-active-gait');
            if (ag) ag.textContent = d.active_gait.toUpperCase();
            const hag = document.getElementById('hud-active-gait');
            if (hag) hag.textContent = d.active_gait.toUpperCase();
            const sag = document.getElementById('side-gait-status');
            if (sag) sag.textContent = d.active_gait.toUpperCase();
          }
          if (d.pca_found !== undefined) {
            const elPca = document.getElementById('hud-pca-status');
            const elPcaBadge = document.getElementById('hud-pca-badge');
            if (elPca) {
              elPca.innerText = d.pca_found ? 'PCA9685: READY (0x40)' : 'PCA9685: OFFLINE';
            }
            if (elPcaBadge) {
              elPcaBadge.style.color = d.pca_found ? 'var(--accent-sky)' : 'var(--accent-rose)';
            }
          }
          if (d.weight_l !== undefined) {
            const wl = Math.round(d.weight_l);
            const elWl = document.getElementById('weight-val-l');
            if (elWl) elWl.innerText = wl + 'g';
            const pctL = Math.min(100, Math.round((d.weight_l / 500) * 100));
            const elPctL = document.getElementById('weight-pct-l');
            if (elPctL) elPctL.innerText = pctL + '%';
            const elBarL = document.getElementById('weight-bar-l');
            if (elBarL) elBarL.style.width = pctL + '%';
            const pfL = document.getElementById('plate-foot-l');
            if (pfL) pfL.setAttribute('opacity', (0.35 + (pctL / 100) * 0.65).toFixed(2));
          }
          if (d.weight_r !== undefined) {
            const wr = Math.round(d.weight_r);
            const elWr = document.getElementById('weight-val-r');
            if (elWr) elWr.innerText = wr + 'g';
            const pctR = Math.min(100, Math.round((d.weight_r / 500) * 100));
            const elPctR = document.getElementById('weight-pct-r');
            if (elPctR) elPctR.innerText = pctR + '%';
            const elBarR = document.getElementById('weight-bar-r');
            if (elBarR) elBarR.style.width = pctR + '%';
            const pfR = document.getElementById('plate-foot-r');
            if (pfR) pfR.setAttribute('opacity', (0.35 + (pctR / 100) * 0.65).toFixed(2));
          }
          if (d.force_n !== undefined) {
            const elF = document.getElementById('hud-force-n');
            if (elF) elF.innerText = d.force_n.toFixed(2) + ' N';
          }
          if (d.contact) {
            const elC = document.getElementById('hud-contact-state');
            if (elC) elC.innerText = d.contact;
          }
          if (d.heap !== undefined) {
            const elH = document.getElementById('stat-heap');
            if (elH) elH.innerText = Math.round(d.heap / 1024) + ' KB';
          }
          if (d.uptime !== undefined) {
            const elU = document.getElementById('stat-uptime');
            if (elU) {
              const sec = d.uptime;
              const hrs = Math.floor(sec / 3600);
              const mins = Math.floor((sec % 3600) / 60);
              const secs = sec % 60;
              elU.innerText = (hrs < 10 ? '0' : '') + hrs + ':' +
                              (mins < 10 ? '0' : '') + mins + ':' +
                              (secs < 10 ? '0' : '') + secs;
            }
          }
          if (d.rssi !== undefined) {
            const elR = document.getElementById('stat-rssi');
            if (elR) elR.innerText = (d.rssi === 0 ? 'SoftAP' : (d.rssi + ' dBm'));
          }
          if (d.ip) {
            const elIp = document.getElementById('stat-ip');
            if (elIp) elIp.innerText = d.ip;
          }
        }).catch(()=>{});
    }

    setInterval(pollTelemetry, 350);
    pollTelemetry();
    selectJoint(0, 'Right Hip');
    switchSchematic('side');
    animateRig(liveAngles);
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H

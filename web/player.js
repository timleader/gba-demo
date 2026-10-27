"use strict";

(() => {
    const screen = document.getElementById("screen");
    const play = document.getElementById("play");
    const restart = document.getElementById("restart");
    const mute = document.getElementById("mute");
    const status = document.getElementById("status");
    const warning = document.getElementById("warning");
    const romPath = new URLSearchParams(location.search).get("rom") || "../gba-demo.gba";
    let gba;
    let rom;
    let state = "loading";

    function fail(error) {
        console.error(error);
        if (state === "error") {
            return;
        }
        state = "error";
        if (gba) {
            gba.pause();
            gba.keypad.eatInput = false;
        }
        play.disabled = restart.disabled = mute.disabled = true;
        status.dataset.error = "true";
        status.textContent = `${error.message || error} Reload this page to try again.`;
    }

    function pause() {
        if (state !== "playing") {
            return;
        }
        gba.pause();
        gba.keypad.currentDown = 0x03ff;
        gba.keypad.eatInput = false;
        state = "paused";
        play.textContent = "Resume";
        status.textContent = "Paused.";
    }

    async function start(reset) {
        play.disabled = restart.disabled = true;
        try {
            if (gba.audio.context) {
                await gba.audio.context.resume();
            }
            if (state === "error") {
                return;
            }
            if (reset) {
                gba.pause();
                if (!gba.setRom(rom)) {
                    throw new Error(`gbajs2 could not load ${romPath}.`);
                }
            }
            if (state === "error") {
                return;
            }
            gba.keypad.currentDown = 0x03ff;
            gba.keypad.eatInput = true;
            gba.runStable();
            state = "playing";
            play.textContent = "Pause";
            status.textContent = "Playing.";
            screen.focus();
            play.disabled = restart.disabled = false;
        } catch (error) {
            fail(error);
        }
    }

    play.addEventListener("click", () => {
        if (state === "playing") {
            pause();
        } else {
            void start(false);
        }
    });
    restart.addEventListener("click", () => void start(true));
    mute.addEventListener("click", () => {
        const muted = mute.getAttribute("aria-pressed") !== "true";
        gba.audio.masterVolume = muted ? 0 : 1;
        mute.setAttribute("aria-pressed", String(muted));
        mute.textContent = muted ? "Unmute" : "Mute";
    });
    window.addEventListener("blur", pause);
    document.addEventListener("visibilitychange", () => {
        if (document.hidden) {
            pause();
        }
    });

    async function load() {
        try {
            if (location.protocol === "file:") {
                throw new Error("Serve the repository over HTTP; file:// cannot load the ROM or renderer.");
            }
            gba = new GameBoyAdvance();
            gba.pause();
            gba.setLogger((level, message) => {
                if (level === gba.LOG_ERROR) {
                    fail(new Error(`Emulator error: ${message}`));
                } else {
                    console.warn(message);
                    warning.textContent = `Emulator warning: ${message}`;
                    warning.hidden = false;
                }
            });
            gba.setCanvas(screen);
            gba.setBios(biosBin);
            gba.keypad.KEYCODE_A = 88;
            gba.keypad.KEYCODE_B = 90;
            gba.keypad.KEYCODE_L = 81;
            gba.keypad.KEYCODE_R = 69;
            gba.keypad.KEYCODE_SELECT = 8;
            if (gba.video.renderPath.worker) {
                gba.video.renderPath.worker.addEventListener("error", () => {
                    fail(new Error("The gbajs2 renderer failed to load or run. Check the web/js/video files."));
                });
            }
            const response = await fetch(romPath, { cache: "no-store" });
            if (!response.ok) {
                throw new Error(`Could not load ${romPath} (HTTP ${response.status}). Build the ROM with make first.`);
            }
            rom = await response.arrayBuffer();
            if (rom.byteLength < 0xc0 || new DataView(rom).getUint8(0xb2) !== 0x96) {
                throw new Error(`${romPath} is not a valid GBA ROM.`);
            }
            if (state === "error") {
                return;
            }
            if (!gba.setRom(rom)) {
                throw new Error(`gbajs2 could not load ${romPath}.`);
            }
            if (state === "error") {
                return;
            }
            state = "ready";
            status.textContent = `Ready - ${romPath}. Click Play to start.`;
            play.disabled = restart.disabled = false;
            mute.disabled = !gba.audio.context;
            if (!gba.audio.context) {
                warning.textContent = "Web Audio is unavailable. The demo will play without sound.";
                warning.hidden = false;
            }
        } catch (error) {
            fail(error);
        }
    }

    void load();
})();

# AIO-Switch-Updater-Reborn

<p align="center">
  <img src = "icon.jpg"><br>
</p>

<p align="center">
  <img src="https://img.shields.io/github/downloads/exploitz86/aio-switch-updater-reborn/total" alt="downloads">
  <a href="https://github.com/exploitz86/aio-switch-updater-reborn/actions/workflows/main.yml">
    <img src="https://github.com/exploitz86/aio-switch-updater-reborn/actions/workflows/main.yml/badge.svg" alt="Build">
  </a>
  <img src="https://img.shields.io/github/v/release/exploitz86/aio-switch-updater-reborn" alt="release">
  <img src="https://img.shields.io/github/license/exploitz86/aio-switch-updater-reborn" alt="license">
</p>

<p align="center">
  <a href="https://ko-fi.com/exploitz">
    <img height="50" src="https://storage.ko-fi.com/cdn/brandasset/kofi_button_red.png" alt="Support me on Ko-fi">
  </a>
</p>

---

A Nintendo Switch homebrew app to download and update CFWs, FWs, cheat codes, and mods. Supports Atmosphère, ReiNX and SXOS.

**This is a fork of the awesome AIO Switch Updater by HamletDuFromage.** It was initially created for personal use with enhanced features and reliability. All repositories used by this app have been forked to ensure everything works as intended. If you find this useful, please consider donating to help keep the server running for the cheat database updates. Also, please consider donating to HamletDuFromage for the original work!

Works on **unpatched** (Erista) and **patched** (v2/Mariko) Switches.

## How to install
Copy the `aio-switch-updater-reborn/` directory to `/switch/` on your sdcard.

## Features
### ⬦ Update CFW
- Update the Atmosphère Switch Custom Firmware. AIO-Switch-Updater-Reborn uses a custom RCM payload to finalise the install as it can't be performed while HOS is running.
  - If you would like to preserve additional files or directories, write their path (one line each) in `/config/aio-switch-updater-reborn/preserve.txt` and they won't be overwritten when updating.
  - Place [this file](https://github.com/exploitz86/aio-switch-updater-reborn/blob/master/copy_files.txt) in `/config/aio-switch-updater-reborn/copy_files.txt` in order to have specific copy operations performed after each download. This is mainly meant for users with trinkets who want payloads automatically copied to a directory.

### ⬦ Update Hekate/Payload
- Download and update Hekate, as well as a selection of RCM payloads

### ⬦ Custom Downloads
- A custom Atmosphère url can be entered in [this file](https://github.com/exploitz86/aio-switch-updater-reborn/blob/master/custom_packs.json). Once moved to `/config/aio-switch-updater-reborn/custom_packs.json`, it will show on the `Custom Download` menu. This can be used to support third-party packs through AIO-Switch-Updater-Reborn. Packs whose name starts by `[PACK]` won't prompt for a Hekate download.
- Non-Atmosphère downloads can also be added in the `misc` category.

### ⬦ Download firmwares
- Download firmware files to `/firmware` that can then be installed using DayBreak.

### ⬦ Download cheats
- Download and extract daily-updated cheat codes. The program will only extract cheat codes for the games you own. By default, this homebrew will overwrite the existing cheats. If you have your own cheat files that you'd like to keep as is, you can turn off cheat updates for specific titles in `Tools→Cheat Menu`.
- Download cheat sheets from [Cheat Slips](https://www.cheatslips.com/).
- Download individual cheat codes from the `GBAtemp.net` database.
- Includes popular 60FPS cheats created by ChanseyIsTheBest.

### ⬦ Mod Manager & Downloader
- **Browse and Download Mods**: Search and download mods directly from GameBanana for your games with fast access to a massive library of community-made enhancements, tweaks, and custom content.
- **Manage Installed Mods**: Enable/disable mods, view mod details, and uninstall mods with an intuitive interface.
- **Mod Presets**: Create, edit, and apply mod presets to quickly switch between different mod configurations.
- **Automatic Mod Status Checking**: The app tracks which mods are active, inactive, or partially applied.
- **Game-Specific Mod Management**: Mods are automatically organized by game title ID for easy management.

## Extras (in the `Tools` tab)
- Reboot to specific payload.
- Check for games with missing updates.
- Change software color scheme of Joy-Cons. Additional color profiles can be found in the releases and should be copied to `config/aio-switch-updater-reborn/jc_profiles.json`. Use [this webpage](https://hamletdufromage.github.io/JC-color-picker/JCpicker.html) to generate your own profiles.
- Change software color scheme of Pro Controllers (has to be paired as Player 1). Additional color profiles can be found in the releases and should be copied to `config/aio-switch-updater-reborn/pc_profiles.json`.
- View installed cheat codes.
- Launch the Switch's web browser.
- Edit internet settings (DNS, IP address, MTU, etc). Add your own configs to `config/aio-switch-updater-reborn/internet.json`. You can find a template in the root of the repo.
- Clean up downloaded files and atmosphere report folders (crash_reports, fatal_reports, fatal_errors, erpt_reports).
- Hide tabs through the `Hide tabs` menu. More entries can be hidden by manually editing [`config/aio-switch-updater-reborn/hide_tabs.json`](https://github.com/exploitz86/aio-switch-updater-reborn/blob/master/hide_tabs.json).

## Screenshots
<details><summary>Expand to view the screenshots</summary>

![ams_tab](https://user-images.githubusercontent.com/61667930/193625554-ad9a8a5a-72ad-462e-95d9-94979c9750ac.jpg)
![cheats_tab](https://user-images.githubusercontent.com/61667930/193625551-9912210a-c99c-434f-ab5e-b468a698ddcf.jpg)
![individual_cheats](https://user-images.githubusercontent.com/61667930/193625547-18bff50c-1985-4ce5-aadf-2394fa5d29ca.jpg)
![tools_tab](https://user-images.githubusercontent.com/61667930/193625542-4722690a-a86f-48d1-8935-367b16f355f8.jpg)

</details>

## Build

<details><summary>Expand to view the build instructions</summary>

You need to have installed devkitPro and devkitARM in order to compile this project.

Install the required dependencies:
```bash
$ sudo (dkp-)pacman -Sy
```
```bash
$ sudo (dkp-)pacman -S  switch-glfw \
                        switch-curl \
                        switch-glad \
                        switch-glm \
                        switch-mbedtls \
                        switch-zlib \
                        devkitarm-rules
```
Use [`switch-ex-curl`](https://github.com/eXhumer/switch-ex-curl) instead of `switch-curl` to use this app with an invalid SSL certificate.

Clone the repository
```bash
$ git clone --recursive https://github.com/exploitz86/aio-switch-updater-reborn
$ cd aio-switch-updater-reborn
```

Compile 
```bash
$ cd aiosu-forwarder
$ make
$ cd ..
$ make
```

</details>

## Contribute

PRs and suggestions are encouraged! If you wish to help with the localization of the app, you can translate the files in `resources/i18n/`. To easily find the non-translated strings and translate them, you may use `localizer.py` (e.g. `python localizer.py -r resources//i18n//en-US//menus.json -w resources//i18n//fr//menus.json`).

## Disclaimer
I do not own, host nor distribute any of the files that can be downloaded with this homebrew tool. At the owner's request, I will immediately remove the ability to download any problematic file.

## Special thanks
- [HamletDuFromage](https://github.com/HamletDuFromage) for the original code of this app.
- [nadrino](https://github.com/nadrino) for the SimpleModManager and the modified Borealis library.
- [PoloNX](https://github.com/PoloNX) for the SimpleModDownloader.
- [tiansongyu](https://github.com/tiansongyu) for bringing support for multi-language and for his Chinese translation.
- [yyoossk](https://github.com/yyoossk) for the Japanese locale.
- [sergiou87](https://github.com/sergiou87) for the Spanish locale.
- [pedruhb](https://github.com/pedruhb) for the Brazilian locale.
- [AD2076](https://github.com/AD2076) for the Italian locale.
- [qazrfv1234](https://github.com/qazrfv1234) for the Traditional Chinese locale.
- [Nota Inutilis](https://github.com/NotaInutilis/) for the French locale.
- [Team Neptune](https://github.com/Team-Neptune) whose rcm code I used.
- [fennectech](https://github.com/fennectech) for helping test the app and providing suggestions.
- [ChanseyIsTheBest](https://github.com/ChanseyIsTheBest) for the popular 60FPS cheats.
- Iliak for [Cheat Slips](https://www.cheatslips.com/).

### Like the app? originally it has been made by HamletDuFromage. Please consider donating to him!

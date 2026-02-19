# GhbdtnFix
**Instantly fix text typed in the wrong keyboard layout — one hotkey away.**

## What it does

LayoutFixer helps you correct text when you accidentally typed in the wrong keyboard layout.

Classic example:  
You wanted to write `Hello, World!`  
But your keyboard was set to Russian -> you got `Руддщ, Цщкд!`  

Select the messed-up text anywhere (browser, editor, chat, Notepad, etc), press your hotkey -> it instantly becomes correct.

Also includes quick text cleanup tools:
- Remove extra spaces inside words (`Lon don` -> `London`)
- Convert entire selection to lowercase
- Convert entire selection to UPPERCASE

## Features & Default Hotkeys

| Action                              | Default Hotkey         | Example Before → After                              |
|-------------------------------------|------------------------|-----------------------------------------------------|
| Fix keyboard layout (RU ↔ EN)       | `Ctrl + 9`     | `Ghbdtn, World!` -> `Привет, World!`                |
| Remove extra spaces in words        | `Ctrl + 0`     | `Lon don is gr eat` -> `Londonisgreat`              |
| Invert the case                     | `Ctrl + ;`     | `HeLLo, WOrLd` -> `hEllO, woRlD`                    |
| Convert to lowercase                | `Ctrl + [`     | `HeLLo WoRLD!` -> `hello world!`                    |
| Convert to UPPERCASE                | `Ctrl + ]`     | `hello world!` -> `HELLO WORLD!`                    |

*All hotkeys will be fully customizable in future when GUI will be made.*

## How it works (under the hood)

1. You select any text using mouse drag
2. Press the hotkey  
3. App simulates `Ctrl+C` (copies selection to clipboard)  
4. Reads the clipboard content  
5. Applies the chosen transformation  
6. Pastes the fixed text back (replaces selection)  
7. Clears the clipboard for privacy

The whole process usually takes < 1 second.

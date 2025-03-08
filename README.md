
# CS2 Kernel-Level Game Enhancer

This is an open-source, external, read-only kernel-level gameplay enhancer for Counter-Strike 2.

## What does this mean?

-   **Open Source**: The entire project's source code is publicly available, allowing anyone to access, review, and learn from it.
-   **External**: This program operates completely outside the CS2 process and doesn't rely on Windows API functions to access the game.
-   **Read-Only**: We don't write anything to the target process memory, eliminating the risks associated with memory manipulation.
-   **Kernel-Level**: The program uses a kernel driver to achieve maximum stealth and hide itself from user-mode detection systems.

## Purpose

This project was created solely for educational purposes, intended for those interested in learning about Windows kernel development and kernel-level memory reading/writing techniques.


# How to Use

1.  Download all files from the latest release.
2.  Load the kernel driver:
    -   Drag and drop `driver.sys` onto `kdmapper.exe`
    -   Wait for confirmation that the driver has been successfully loaded
3.  Launch Counter-Strike 2 in fullscreen winowed or window mode and wait until you're in the lobby.
4.  Start `client.exe` to activate the enhancer.
5. Press `home` button on your keyboard to activate menu

 ### Common issues
 if you have problem with loading driver follow this steps:
 - disable Core Isolation in windows defender
 -  got to `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\CI\Config` in regedit and set `VulnerableDriverBlocklistEnable` to 0

# Features
- ESP
	- Box
	- Bone
	- Guns
	- Health
	- Ammo 
	- Granades
	- Defuse Kit
- Bomb Info
	- Timer
	- Site
	- Defuse Time
- Aimbot
	- smoothing 
	- Target Position
	- Recoil Control
- Misc
	- Recoil Dot
 	- Stream proof 
	- Text Sizes
	- FPS Control

### TODO
- Ranks
- Spectator List
- Trigger Bot
- Better Aimbot & ESP
  
## Showcase

<div align="center">
  <img src="https://github.com/PicoShot/PicoDriver/blob/main/showcase/1.png" alt="Screenshot 1" width="32%" />
  <img src="https://github.com/PicoShot/PicoDriver/blob/main/showcase/2.png" alt="Screenshot 2" width="32%" />
  <img src="https://github.com/PicoShot/PicoDriver/blob/main/showcase/3.png" alt="Screenshot 3" width="32%" />
</div>

# VAC
This project operates at the kernel level, while Valve Anti-Cheat (VAC) functions primarily at the user-mode level. Due to this architectural difference:

VAC cannot directly detect our kernel-mode driver
The enhancer remains undetected by VAC's standard scanning methods
Our read-only approach minimizes detection vectors typically associated with memory manipulation

Important Notice: While the current implementation is designed to avoid detection, Valve continuously updates their anti-cheat systems. We cannot guarantee permanent undetectability, and users should proceed with caution.
This information is provided for educational purposes only. Users are responsible for understanding and accepting the risks associated with using any third-party software that interacts with online games.

# Compile it yourself!

## Requirements

-   Visual Studio 2022
-   Windows Driver Kit (WDK) - latest version recommended
-   Windows SDK 10 or newer
-   C++ Desktop Development workload for Visual Studio

## Setup Instructions

1.  **Install Visual Studio 2022**
    -   Download from [Visual Studio website](https://visualstudio.microsoft.com/vs/)
    -   During installation, select the "Desktop development with C++" workload
2.  **Install Windows Driver Kit (WDK)**
    -   Download the latest WDK from [Microsoft's WDK download page](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
    -   Install the WDK following Microsoft's installation guide
    -   Ensure you install the matching Windows SDK version if prompted
3.  **Clone the Repository**
    
    Copy
    
    `git clone https://github.com/PicoShot/PicoDriver.git`
    
4.  **Open the Solution**
    -   Open the `.sln` file with Visual Studio 2022
    -   Ensure both the kernel driver and client projects are loaded
5.  **Build the Solution**
    -   Select the desired configuration (Release is recommended for usage)
    -   Build the entire solution (F7 or Build → Build Solution)
    -   Output files will be located in the `Build/` directory

# Help

you can reach me on [discord](https://discord.com/users/1208419597792055417) for any help 

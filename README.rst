============================
Vault-Tec Tactical Assistant
============================

Providing you tactical aid during skirmishes with other wastelanders.

Features
________

* Hexbot: make sure that no bullet is wasted and you wreak maximum havoc with your minigun.
	demo https://gfycat.com/AchingVeneratedGodwit

* ... more tactical aids in the future!
	
Download
________

Look at the `Releases`_ page.

.. _Releases: https://github.com/EnglishBreakfastTea/Vault-Tec-Tactical-Assistant/releases
	
Usage
_____

| Move the files Injector.exe, libVaultTecTacticalAssistant.dll, VaultTecTacticalAssistantUI.exe and vttainit.ini (optional) to your FOnline directory. 
| Run Injector.exe - your game should start normally.
| After the game starts, run VaultTecTacticalAssistantUI. A console should show up. This console is how you speak to your assistant.
| Use the console to give commands. If vttainit.ini is present in the FOnline directory, commands written there will be executed automatically when starting the UI.
| The following commands are available:

  * toggle 1hex - enable/disable hexbot v1
  * toggle 1hexv2 - enable/disable hexbot v2
  * center - center the view on the player

| A text in the upper left corner of your game window should indicate that the hexbot is enabled.
| A special command is available for assigning commands to hotkeys:

	bind [c][a][s]_key "command"
		where 'c', 'a', 's' are optional, 'key' is a letter, 'command' is the chosen command.
		'c' is a ctrl modifier, 'a' is an alt modifier, 'shift' is a shift modifier.
	For example
		bind c_v "toggle 1hexv2"

	will assign the "toggle 1hexv2" command to the "Ctrl+V" hotkey.

| The difference between hexbot v1 and v2 is explained on the Releases page.
| The release contains a "vttainit.ini" file with some commands already assigned. Adjust to your liking.

Compiling
_________

| You need CMake and a modern C++ compiler (supporting the C++14 standard). Also the `Boost`_ library version 1.61 is required (only the header-only part). Modify the CMakeLists.txt file to adjust your Boost path. I compile this project with `MinGW g++`_ - you may need to change the "-std=gnu++14" compiler flag to "-std=c++14" when using different compilers.
| After you get the required tools... no, I will not write another tutorial for using CMake.

.. _Boost: http://www.boost.org/
.. _MinGw g++: https://sourceforge.net/projects/mingw/

Development
___________

I will be happy for any contributions. If you have any interesting idea, implement it and make a pull request. For a start, read the notes (in polish) in the notatki.txt file. It contains some information that I gathered when reverse engineering the game.

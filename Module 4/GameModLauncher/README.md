# Steam Mod Manager (C++ and SQLite)

## What changed
This version replaces the previous `games.txt` and `mods.txt` storage with a relational SQLite database named `steam_mod_manager.db`. Records are written to the database immediately, so there is no separate save step.

## Features
- Detect installed Steam games from the Windows registry and Steam library manifests.
- Add, view, edit, and delete game records.
- Add, view, edit, and delete mod records.
- Associate every mod with a game through a foreign key.
- Toggle the stored enabled/disabled status of a mod.
- List mods with their game names using an SQL `INNER JOIN`.
- Delete a game and automatically delete its associated mod records using `ON DELETE CASCADE`.

## Database design
The application creates `Games` and `Mods` tables the first time it runs. `Games.game_id` is the primary key. `Mods.mod_id` is the primary key, and `Mods.game_id` is a foreign key referencing `Games.game_id`. The relationship is one game to many mods.

The app uses SQL `INSERT`, `SELECT`, `UPDATE`, and `DELETE` statements. It uses prepared statements and bound parameters for user-entered values. The `getModsWithGames()` method runs an `INNER JOIN` between the two tables.

## Build in Visual Studio 2022
This project uses the vcpkg manifest to obtain SQLite's C/C++ library. The SQLite dependency is not a separate database server; SQLite stores everything in one local `.db` file.

1. Install Visual Studio 2022 with the **Desktop development with C++** workload.
2. Install vcpkg following Microsoft's official vcpkg setup instructions: https://learn.microsoft.com/vcpkg/get_started/get-started
3. Run `vcpkg integrate install` from the vcpkg directory in a Developer PowerShell or Command Prompt.
4. Open `GameModLauncher.sln` from this folder.
5. Select **x64** and **Debug**.
6. Build the solution. With manifest mode enabled, vcpkg reads `vcpkg.json` and installs the `sqlite3` dependency for the `x64-windows-static` triplet. The first build needs internet access to download/build the dependency.
7. Run the program. It creates `steam_mod_manager.db` in the application's working directory.

If Visual Studio does not restore the dependency automatically, run `vcpkg install --triplet x64-windows-static` from this folder after configuring vcpkg, then reopen the solution.

## Data migration note
The SQLite version creates a new database file. Existing `games.txt` and `mods.txt` files from the earlier version are not automatically imported. Keep a backup of those text files if you have data you want to preserve.

## Suggested demonstration/test
1. Run the app and choose **11. Load example data**.
2. Choose **1. View games** and **6. View mods**. The mod listing retrieves the game name through an SQL `INNER JOIN`.
3. Add a second game and add a mod to it.
4. Edit the mod, toggle its status, and remove it.
5. Remove a game and confirm that its associated mod records are deleted by the foreign-key cascade.
6. Exit and reopen the app. The records should remain in `steam_mod_manager.db`.

## Important limitation
The enabled/disabled value is currently a database status flag. It does not physically install, remove, or activate mod files in a game. The app also records mod download pages and local paths but does not download mods.

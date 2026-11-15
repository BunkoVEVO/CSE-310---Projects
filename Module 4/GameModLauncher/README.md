# Steam Mod Manager

## Overview

Steam Mod Manager is a C++ application designed to help organize Steam games and manage information about their mods. The program uses an SQLite relational database to store game and mod information instead of relying on separate text files.

The application can scan a Steam installation to find installed games and save them to the database. Users can also manually add, edit, and delete games. Mods can be associated with individual games, and users can add, edit, delete, and toggle the enabled status of mods.

The purpose of this project is to improve my skills as a software engineer by learning how to integrate a relational database into a C++ application. I wanted to create something useful based on my interest in video games and modding while gaining experience with SQL, database relationships, and application development.

### How to Use the Program

1. Build and run the application using Visual Studio.
2. The program creates an SQLite database named `steam_mod_manager.db` when it starts.
3. Choose the option to scan Steam to find installed games.
4. Add games manually if they are not discovered automatically.
5. Add mods and associate them with the appropriate games.
6. Use the menu options to view, edit, delete, or change the enabled status of games and mods.

The program stores its information in the SQLite database, so the records remain available when the application is closed and reopened.

**Note:** The enabled/disabled feature currently tracks a mod's status in the database. It does not physically install, uninstall, or activate mod files inside a game's directory.

[Software Demo Video](https://youtu.be/8U_PKObqNAU)

## Relational Database

This project uses **SQLite**, a lightweight relational database that stores information locally in a database file. SQLite allows the C++ application to execute SQL commands without requiring a separate database server.

The database is stored in a file named `steam_mod_manager.db`. It contains two main tables: `Games` and `Mods`.

### Database Tables

#### Games

The `Games` table stores information about games managed by the application.

| Column | Data Type | Description |
|---|---|---|
| `game_id` | INTEGER | Primary key that uniquely identifies each game. |
| `name` | TEXT | Name of the game. |
| `install_path` | TEXT | Location where the game is installed. |
| `app_id` | TEXT | Steam application ID. |

#### Mods

The `Mods` table stores information about mods associated with games.

| Column | Data Type | Description |
|---|---|---|
| `mod_id` | INTEGER | Primary key that uniquely identifies each mod. |
| `game_id` | INTEGER | Foreign key connecting the mod to a game. |
| `name` | TEXT | Name of the mod. |
| `source` | TEXT | Website or source where the mod came from. |
| `page_url` | TEXT | Link to the mod's webpage. |
| `local_file` | TEXT | Local location of the mod file. |
| `enabled` | INTEGER | Stores whether the mod is enabled (1) or disabled (0). |

### Table Relationships

The database uses a **one-to-many relationship** between `Games` and `Mods`.

One game can have multiple mods, but each mod belongs to one game. The `game_id` column in the `Mods` table is a foreign key referencing the `game_id` primary key in the `Games` table.

The foreign key uses `ON DELETE CASCADE`, which means deleting a game also deletes the mods associated with it.

The application also uses an SQL `INNER JOIN` to retrieve mod information together with the name of the game each mod belongs to. This demonstrates how relational database tables can be connected to retrieve related information.

The program uses SQL commands for the main database operations:

- **INSERT** – Adds games and mods.
- **SELECT** – Retrieves games and mods.
- **UPDATE** – Changes existing game and mod information.
- **DELETE** – Removes games and mods.
- **INNER JOIN** – Combines information from the `Games` and `Mods` tables.

## Development Environment

The application was developed using the following tools:

- **Visual Studio 2026** – Used to write, build, and debug the

-- Steam Mod Manager relational database schema
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Games (
    game_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    install_path TEXT NOT NULL DEFAULT '',
    app_id TEXT NOT NULL DEFAULT ''
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_games_app_id
ON Games(app_id) WHERE app_id <> '';

CREATE TABLE IF NOT EXISTS Mods (
    mod_id INTEGER PRIMARY KEY AUTOINCREMENT,
    game_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    source TEXT NOT NULL DEFAULT '',
    page_url TEXT NOT NULL DEFAULT '',
    local_file TEXT NOT NULL DEFAULT '',
    enabled INTEGER NOT NULL DEFAULT 0 CHECK(enabled IN (0, 1)),
    FOREIGN KEY(game_id) REFERENCES Games(game_id) ON DELETE CASCADE
);

-- Example relational JOIN used by the application to list each mod with its game.
SELECT Mods.mod_id, Mods.name AS mod_name, Games.name AS game_name,
       Mods.source, Mods.page_url, Mods.local_file, Mods.enabled
FROM Mods
INNER JOIN Games ON Mods.game_id = Games.game_id
ORDER BY Games.name, Mods.name;

#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <regex>
#include <iterator>
#include <windows.h>
#include <sqlite3.h>

using namespace std;

const char* DATABASE_FILE = "steam_mod_manager.db";

string askLine(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

int askNumber(const string& prompt, int minimum, int maximum) {
    while (true) {
        cout << prompt;
        int value;
        if (cin >> value && value >= minimum && value <= maximum) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Enter a number from " << minimum << " to " << maximum << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

class Game {
public:
    int id;
    string name, installPath, appId;

    Game(int gameId, const string& gameName, const string& path, const string& steamId)
        : id(gameId), name(gameName), installPath(path), appId(steamId) {}

    void display() const {
        cout << "Game ID: " << id << "\n"
             << "Name: " << name << "\n"
             << "Location: " << (installPath.empty() ? "Not provided" : installPath) << "\n"
             << "Steam App ID: " << (appId.empty() ? "Not provided" : appId) << "\n";
    }
};

class Mod {
public:
    int id, gameId;
    string name, gameName, source, pageUrl, localFile;
    bool enabled;

    Mod(int modId, int parentGameId, const string& modName, const string& parentGame,
        const string& website, const string& url, const string& file, bool isEnabled)
        : id(modId), gameId(parentGameId), name(modName), gameName(parentGame),
          source(website), pageUrl(url), localFile(file), enabled(isEnabled) {}

    void display() const {
        cout << "Mod ID: " << id << "\n"
             << "Mod: " << name << "\n"
             << "Game: " << gameName << " (ID " << gameId << ")\n"
             << "Source: " << (source.empty() ? "Not provided" : source) << "\n"
             << "Page: " << (pageUrl.empty() ? "Not provided" : pageUrl) << "\n"
             << "Local file: " << (localFile.empty() ? "Not recorded" : localFile) << "\n"
             << "Status: " << (enabled ? "Enabled" : "Disabled") << "\n";
    }
};

// Owns the SQLite connection and all database operations.
class Database {
private:
    sqlite3* db;

    bool prepare(const string& sql, sqlite3_stmt** statement) const {
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, statement, NULL);
        if (result != SQLITE_OK) {
            cerr << "SQL error: " << sqlite3_errmsg(db) << "\n";
            return false;
        }
        return true;
    }

    void bindText(sqlite3_stmt* statement, int position, const string& value) const {
        sqlite3_bind_text(statement, position, value.c_str(), -1, SQLITE_TRANSIENT);
    }

public:
    Database() : db(NULL) {
        int result = sqlite3_open(DATABASE_FILE, &db);
        if (result != SQLITE_OK) {
            cerr << "Could not open database: " << sqlite3_errmsg(db) << "\n";
            if (db) sqlite3_close(db);
            db = NULL;
            return;
        }

        // Foreign keys must be enabled for each SQLite connection.
        if (!execute("PRAGMA foreign_keys = ON;")) return;

        const char* schema =
            "CREATE TABLE IF NOT EXISTS Games ("
            " game_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " name TEXT NOT NULL UNIQUE,"
            " install_path TEXT NOT NULL DEFAULT '',"
            " app_id TEXT NOT NULL DEFAULT ''"
            ");"
            "CREATE UNIQUE INDEX IF NOT EXISTS idx_games_app_id "
            "ON Games(app_id) WHERE app_id <> '';"
            "CREATE TABLE IF NOT EXISTS Mods ("
            " mod_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " game_id INTEGER NOT NULL,"
            " name TEXT NOT NULL,"
            " source TEXT NOT NULL DEFAULT '',"
            " page_url TEXT NOT NULL DEFAULT '',"
            " local_file TEXT NOT NULL DEFAULT '',"
            " enabled INTEGER NOT NULL DEFAULT 0 CHECK(enabled IN (0,1)),"
            " FOREIGN KEY(game_id) REFERENCES Games(game_id) ON DELETE CASCADE"
            ");";
        execute(schema);
    }

    ~Database() {
        if (db) sqlite3_close(db);
    }

    bool isOpen() const { return db != NULL; }

    bool execute(const string& sql) const {
        if (!db) return false;
        char* errorMessage = NULL;
        int result = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errorMessage);
        if (result != SQLITE_OK) {
            cerr << "Database error: " << (errorMessage ? errorMessage : sqlite3_errmsg(db)) << "\n";
            sqlite3_free(errorMessage);
            return false;
        }
        return true;
    }

    bool addGame(const string& name, const string& path, const string& appId) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("INSERT INTO Games(name, install_path, app_id) VALUES(?,?,?);", &statement)) return false;
        bindText(statement, 1, name);
        bindText(statement, 2, path);
        bindText(statement, 3, appId);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not add game: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(statement);
        return result == SQLITE_DONE;
    }

    bool updateGame(int id, const string& name, const string& path, const string& appId) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("UPDATE Games SET name=?, install_path=?, app_id=? WHERE game_id=?;", &statement)) return false;
        bindText(statement, 1, name); bindText(statement, 2, path); bindText(statement, 3, appId);
        sqlite3_bind_int(statement, 4, id);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not update game: " << sqlite3_errmsg(db) << "\n";
        bool changed = sqlite3_changes(db) > 0;
        sqlite3_finalize(statement);
        return result == SQLITE_DONE && changed;
    }

    bool deleteGame(int id) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("DELETE FROM Games WHERE game_id=?;", &statement)) return false;
        sqlite3_bind_int(statement, 1, id);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not delete game: " << sqlite3_errmsg(db) << "\n";
        bool changed = sqlite3_changes(db) > 0;
        sqlite3_finalize(statement);
        return result == SQLITE_DONE && changed;
    }

    vector<Game> getGames() const {
        vector<Game> games;
        sqlite3_stmt* statement = NULL;
        if (!prepare("SELECT game_id, name, install_path, app_id FROM Games ORDER BY name;", &statement)) return games;
        while (sqlite3_step(statement) == SQLITE_ROW) {
            games.push_back(Game(sqlite3_column_int(statement, 0),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 3))));
        }
        sqlite3_finalize(statement);
        return games;
    }

    bool gameExists(const string& appId, const string& name) const {
        sqlite3_stmt* statement = NULL;
        if (!prepare("SELECT 1 FROM Games WHERE name=? OR (? <> '' AND app_id=?) LIMIT 1;", &statement)) return false;
        bindText(statement, 1, name); bindText(statement, 2, appId); bindText(statement, 3, appId);
        bool found = sqlite3_step(statement) == SQLITE_ROW;
        sqlite3_finalize(statement);
        return found;
    }

    bool addMod(int gameId, const string& name, const string& source,
                const string& url, const string& file) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("INSERT INTO Mods(game_id,name,source,page_url,local_file,enabled) VALUES(?,?,?,?,?,0);", &statement)) return false;
        sqlite3_bind_int(statement, 1, gameId); bindText(statement, 2, name);
        bindText(statement, 3, source); bindText(statement, 4, url); bindText(statement, 5, file);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not add mod: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(statement);
        return result == SQLITE_DONE;
    }

    bool updateMod(int id, const string& name, const string& source,
                   const string& url, const string& file) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("UPDATE Mods SET name=?, source=?, page_url=?, local_file=? WHERE mod_id=?;", &statement)) return false;
        bindText(statement, 1, name); bindText(statement, 2, source);
        bindText(statement, 3, url); bindText(statement, 4, file);
        sqlite3_bind_int(statement, 5, id);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not update mod: " << sqlite3_errmsg(db) << "\n";
        bool changed = sqlite3_changes(db) > 0;
        sqlite3_finalize(statement);
        return result == SQLITE_DONE && changed;
    }

    bool deleteMod(int id) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("DELETE FROM Mods WHERE mod_id=?;", &statement)) return false;
        sqlite3_bind_int(statement, 1, id);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not delete mod: " << sqlite3_errmsg(db) << "\n";
        bool changed = sqlite3_changes(db) > 0;
        sqlite3_finalize(statement);
        return result == SQLITE_DONE && changed;
    }

    bool toggleMod(int id) {
        sqlite3_stmt* statement = NULL;
        if (!prepare("UPDATE Mods SET enabled = CASE enabled WHEN 1 THEN 0 ELSE 1 END WHERE mod_id=?;", &statement)) return false;
        sqlite3_bind_int(statement, 1, id);
        int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) cerr << "Could not change mod status: " << sqlite3_errmsg(db) << "\n";
        bool changed = sqlite3_changes(db) > 0;
        sqlite3_finalize(statement);
        return result == SQLITE_DONE && changed;
    }

    // This INNER JOIN is the relational query: each mod is returned with its owning game name.
    vector<Mod> getModsWithGames() const {
        vector<Mod> mods;
        const string sql =
            "SELECT Mods.mod_id, Games.game_id, Mods.name, Games.name, "
            "Mods.source, Mods.page_url, Mods.local_file, Mods.enabled "
            "FROM Mods INNER JOIN Games ON Mods.game_id = Games.game_id "
            "ORDER BY Games.name, Mods.name;";
        sqlite3_stmt* statement = NULL;
        if (!prepare(sql, &statement)) return mods;
        while (sqlite3_step(statement) == SQLITE_ROW) {
            mods.push_back(Mod(sqlite3_column_int(statement, 0), sqlite3_column_int(statement, 1),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 3)),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 5)),
                reinterpret_cast<const char*>(sqlite3_column_text(statement, 6)),
                sqlite3_column_int(statement, 7) == 1));
        }
        sqlite3_finalize(statement);
        return mods;
    }
};

class ModManager {
private:
    Database database;

    void displayGameChoices(const vector<Game>& games) const {
        for (size_t i = 0; i < games.size(); ++i)
            cout << i + 1 << ". " << games[i].name << " (ID " << games[i].id << ")\n";
    }

    void addGame() {
        cout << "\n=== ADD GAME ===\n";
        string name = askLine("Game name: ");
        if (name.empty()) { cout << "Name cannot be empty.\n"; return; }
        string path = askLine("Install folder (optional): ");
        string appId = askLine("Steam App ID (optional): ");
        cout << (database.addGame(name, path, appId) ? "Game added to SQLite database.\n" : "Game was not added. Check for duplicates.\n");
    }

    void editGame() {
        vector<Game> games = database.getGames();
        if (games.empty()) { cout << "No games to edit.\n"; return; }
        displayGameChoices(games);
        int index = askNumber("Select game: ", 1, static_cast<int>(games.size())) - 1;
        string name = askLine("New name (Enter keeps current): ");
        string path = askLine("New install path (Enter keeps current): ");
        string appId = askLine("New Steam App ID (Enter keeps current): ");
        if (name.empty()) name = games[index].name;
        if (path.empty()) path = games[index].installPath;
        if (appId.empty()) appId = games[index].appId;
        cout << (database.updateGame(games[index].id, name, path, appId) ? "Game updated.\n" : "Game was not updated.\n");
    }

    void removeGame() {
        vector<Game> games = database.getGames();
        if (games.empty()) { cout << "No games to remove.\n"; return; }
        displayGameChoices(games);
        int index = askNumber("Select game to remove: ", 1, static_cast<int>(games.size())) - 1;
        cout << "This also deletes the game's mod records. Continue? (y/n): ";
        string confirm; getline(cin, confirm);
        if (confirm == "y" || confirm == "Y")
            cout << (database.deleteGame(games[index].id) ? "Game and associated mod records removed.\n" : "Game was not removed.\n");
        else cout << "Cancelled.\n";
    }

    void addMod() {
        vector<Game> games = database.getGames();
        if (games.empty()) { cout << "Add or scan for a game first.\n"; return; }
        cout << "\n=== ADD MOD ===\n";
        displayGameChoices(games);
        int index = askNumber("Choose its game: ", 1, static_cast<int>(games.size())) - 1;
        string name = askLine("Mod name: ");
        if (name.empty()) { cout << "Mod name cannot be empty.\n"; return; }
        string source = askLine("Mod source/site: ");
        string url = askLine("Mod page URL: ");
        string file = askLine("Local mod file path (optional): ");
        cout << (database.addMod(games[index].id, name, source, url, file) ? "Mod added to database.\n" : "Mod was not added.\n");
    }

    void editMod() {
        vector<Mod> mods = database.getModsWithGames();
        if (mods.empty()) { cout << "No mods to edit.\n"; return; }
        showMods();
        int index = askNumber("Select mod number: ", 1, static_cast<int>(mods.size())) - 1;
        string name = askLine("New mod name (Enter keeps current): ");
        string source = askLine("New source (Enter keeps current): ");
        string url = askLine("New page URL (Enter keeps current): ");
        string file = askLine("New local file path (Enter keeps current): ");
        if (name.empty()) name = mods[index].name;
        if (source.empty()) source = mods[index].source;
        if (url.empty()) url = mods[index].pageUrl;
        if (file.empty()) file = mods[index].localFile;
        cout << (database.updateMod(mods[index].id, name, source, url, file) ? "Mod updated.\n" : "Mod was not updated.\n");
    }

    void removeMod() {
        vector<Mod> mods = database.getModsWithGames();
        if (mods.empty()) { cout << "No mods to remove.\n"; return; }
        showMods();
        int index = askNumber("Select mod number to remove: ", 1, static_cast<int>(mods.size())) - 1;
        cout << (database.deleteMod(mods[index].id) ? "Mod removed from database.\n" : "Mod was not removed.\n");
    }

    void toggleMod() {
        vector<Mod> mods = database.getModsWithGames();
        if (mods.empty()) { cout << "No mods available.\n"; return; }
        showMods();
        int index = askNumber("Select mod number: ", 1, static_cast<int>(mods.size())) - 1;
        cout << (database.toggleMod(mods[index].id) ? "Mod status changed.\n" : "Mod status was not changed.\n");
    }

public:
    ModManager() {}

    bool ready() const { return database.isOpen(); }

    void addExampleData() {
        vector<Game> games = database.getGames();
        int dbfzId = -1;
        for (size_t i = 0; i < games.size(); ++i)
            if (games[i].name == "Dragon Ball FighterZ") dbfzId = games[i].id;
        if (dbfzId < 0) {
            database.addGame("Dragon Ball FighterZ", "C:\\Games\\DBFZ", "678950");
            games = database.getGames();
            for (size_t i = 0; i < games.size(); ++i)
                if (games[i].name == "Dragon Ball FighterZ") dbfzId = games[i].id;
        }
        vector<Mod> mods = database.getModsWithGames();
        bool exists = false;
        for (size_t i = 0; i < mods.size(); ++i)
            if (mods[i].name == "Costume Mod" && mods[i].gameId == dbfzId) exists = true;
        if (dbfzId >= 0 && !exists)
            database.addMod(dbfzId, "Costume Mod", "GameBanana", "https://gamebanana.com/", "");
        cout << "Example records are ready.\n";
    }

    void discoverSteamGames() {
        string steamPath;
        HKEY key;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &key) == ERROR_SUCCESS) {
            char buffer[MAX_PATH] = {};
            DWORD size = sizeof(buffer), type = 0;
            if (RegQueryValueExA(key, "SteamPath", NULL, &type,
                reinterpret_cast<LPBYTE>(buffer), &size) == ERROR_SUCCESS) steamPath = buffer;
            RegCloseKey(key);
        }
        if (steamPath.empty()) steamPath = "C:\\Program Files (x86)\\Steam";
        for (size_t i = 0; i < steamPath.size(); ++i) if (steamPath[i] == '/') steamPath[i] = '\\';

        vector<string> libraries(1, steamPath);
        string libraryFile = steamPath + "\\steamapps\\libraryfolders.vdf";
        ifstream libraryIn(libraryFile.c_str());
        string line;
        regex pathPattern("\\\"path\\\"\\s*\\\"([^\\\"]+)\\\"");
        while (getline(libraryIn, line)) {
            smatch match;
            if (!regex_search(line, match, pathPattern)) continue;
            string path = match[1].str();
            size_t pos = 0;
            while ((pos = path.find("\\\\", pos)) != string::npos) { path.replace(pos, 2, "\\"); ++pos; }
            bool duplicate = false;
            for (size_t i = 0; i < libraries.size(); ++i) if (libraries[i] == path) duplicate = true;
            DWORD attr = GetFileAttributesA((path + "\\steamapps").c_str());
            if (!duplicate && attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) libraries.push_back(path);
        }

        int added = 0, manifests = 0;
        for (size_t i = 0; i < libraries.size(); ++i) {
            string folder = libraries[i] + "\\steamapps";
            WIN32_FIND_DATAA data;
            HANDLE handle = FindFirstFileA((folder + "\\appmanifest_*.acf").c_str(), &data);
            if (handle == INVALID_HANDLE_VALUE) continue;
            do {
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                ++manifests;
                string manifestPath = folder + "\\" + data.cFileName;
                ifstream in(manifestPath.c_str());
                if (!in) continue;
                string contents((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
                smatch app, name, install;
                regex appPattern("\\\"appid\\\"\\s*\\\"([^\\\"]+)\\\"");
                regex namePattern("\\\"name\\\"\\s*\\\"([^\\\"]+)\\\"");
                regex installPattern("\\\"installdir\\\"\\s*\\\"([^\\\"]+)\\\"");
                if (!regex_search(contents, app, appPattern) || !regex_search(contents, name, namePattern) ||
                    !regex_search(contents, install, installPattern)) continue;
                string id = app[1].str(), gameName = name[1].str();
                string path = folder + "\\common\\" + install[1].str();
                DWORD attr = GetFileAttributesA(path.c_str());
                if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) continue;
                if (!database.gameExists(id, gameName) && database.addGame(gameName, path, id)) {
                    cout << "Discovered: " << gameName << "\n";
                    ++added;
                }
            } while (FindNextFileA(handle, &data));
            FindClose(handle);
        }
        cout << "\nSteam scan complete. Libraries: " << libraries.size()
             << ", manifests: " << manifests << ", new games added to SQLite: " << added << ".\n";
        if (manifests == 0) cout << "No manifests found. Check your Steam installation and libraries.\n";
    }

    void showGames() const {
        vector<Game> games = database.getGames();
        cout << "\n=== YOUR GAMES (SQLite) ===\n";
        if (games.empty()) { cout << "No games saved.\n"; return; }
        for (size_t i = 0; i < games.size(); ++i) { cout << "\n" << i + 1 << ". "; games[i].display(); }
    }

    void showMods() const {
        vector<Mod> mods = database.getModsWithGames();
        cout << "\n=== YOUR MODS (Games + Mods JOIN) ===\n";
        if (mods.empty()) { cout << "No mods saved.\n"; return; }
        for (size_t i = 0; i < mods.size(); ++i) { cout << "\n" << i + 1 << ". "; mods[i].display(); }
    }

    void run() {
        bool running = true;
        while (running) {
            cout << "\n========== STEAM MOD MANAGER (SQLite) ==========\n"
                 << "Games\n 1. View games       2. Add game\n"
                 << " 3. Edit game        4. Remove game\n"
                 << " 5. Scan Steam\n\nMods\n"
                 << " 6. View mods        7. Add mod\n"
                 << " 8. Edit mod         9. Remove mod\n"
                 << "10. Enable/disable mod\n\n"
                 << "11. Load example data\n12. Exit\n";
            int choice = askNumber("Choose an option: ", 1, 12);
            switch (choice) {
            case 1: showGames(); break;
            case 2: addGame(); break;
            case 3: editGame(); break;
            case 4: removeGame(); break;
            case 5: discoverSteamGames(); break;
            case 6: showMods(); break;
            case 7: addMod(); break;
            case 8: editMod(); break;
            case 9: removeMod(); break;
            case 10: toggleMod(); break;
            case 11: addExampleData(); break;
            case 12: running = false; cout << "Database changes are already saved. Goodbye!\n"; break;
            }
        }
    }
};

int main() {
    ModManager manager;
    if (!manager.ready()) {
        cerr << "The database could not be initialized. Check the SQLite dependency and try again.\n";
        return 1;
    }
    cout << "SQLite database: " << DATABASE_FILE << "\n";
    manager.run();
    return 0;
}

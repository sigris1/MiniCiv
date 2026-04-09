//
// Created by sigris on 24.03.2026.
//

#pragma once

#include "memory"
#include "string"
#include "vector"
#include "unordered_map"
#include "mutex"
#include "atomic"
#include "optional"
#include "cstdint"

struct pg_conn;
struct pg_result;
class GameSession;
class Tile;
class BasicUnit;
class BasicBuilding;
class BasicResource;
class City;
class Player;
class Tribe;

struct GameInfo {
    int id;
    int map_size;
    std::string status;
    int max_players;
    bool is_private;
    int players_count;
};

struct CityDBData {
    int id;
    int main_tile_id;
    int main_tile_x;
    int main_tile_y;
    int tribe_id;
    int size;
    int basic_economic;
    int additional_economic;
    int current_population;
    int unit_count;
    bool advanced_territory;
    float defence_bonus;
};

class GameRepository {
public:
    GameRepository();
    ~GameRepository();

    bool connect(const std::string& connection_string);
    bool isConnected() const noexcept;
    void disconnect();

    void save(const std::shared_ptr<GameSession>& session);
    std::shared_ptr<GameSession> load(int game_id);
    bool deleteGame(int game_id);

    int createGame(int map_size);
    std::optional<int> getGameMapSize(int game_id);
    bool finishGame(int game_id, int winner_tribe_id);

    int saveTribe(int game_id, const std::shared_ptr<Tribe>& tribe);
    std::shared_ptr<Tribe> loadTribe(int game_id, int tribe_id);
    std::vector<std::shared_ptr<Tribe>> loadGameTribes(int game_id);
    bool updateTribeBalance(int game_id, int tribe_id, int balance);
    bool setTribeCapitalCity(int game_id, int tribe_id, int city_id);

    int saveTile(int game_id, const std::shared_ptr<Tile>& tile, int x, int y);
    std::shared_ptr<Tile> loadTile(int game_id, int x, int y);
    std::shared_ptr<Tile> loadTileById(int tile_id);
    std::vector<std::shared_ptr<Tile>> loadGameTiles(int game_id);
    std::vector<std::shared_ptr<Tile>> loadTilesByOwner(int game_id, int tribe_id);
    bool updateTile(int game_id, const std::shared_ptr<Tile>& tile);

    int saveCity(int game_id, const std::shared_ptr<City>& city, int main_tile_id);
    std::shared_ptr<City> loadCity(int game_id, int city_id);
    std::vector<std::shared_ptr<City>> loadTribeCities(int game_id, int tribe_id);
    bool updateCity(int game_id, const std::shared_ptr<City>& city);
    bool deleteCity(int game_id, int city_id);

    int saveUnit(int game_id, const std::shared_ptr<BasicUnit>& unit);
    std::shared_ptr<BasicUnit> loadUnit(int game_id, int tribe_id, int x, int y);
    std::vector<std::shared_ptr<BasicUnit>> loadTribeUnits(int game_id, int tribe_id);
    std::vector<std::shared_ptr<BasicUnit>> loadTileUnits(int tile_id);
    bool updateUnit(int game_id, const std::shared_ptr<BasicUnit>& unit);
    bool deleteUnit(int game_id, int tribe_id, int x, int y);

    int saveBuilding(int game_id, const std::unique_ptr<BasicBuilding>& building, int tile_id);
    std::shared_ptr<BasicBuilding> loadBuilding(int game_id, int building_index);
    std::vector<std::shared_ptr<BasicBuilding>> loadTileBuildings(int game_id);
    std::vector<std::shared_ptr<BasicBuilding>> loadCityBuildings(int game_id, int city_id);
    bool updateBuilding(int game_id, const std::shared_ptr<BasicBuilding>& building);
    bool deleteBuilding(int game_id, int building_index);

    int saveResource(int game_id, const std::unique_ptr<BasicResource>& resource, int tile_id);
    std::shared_ptr<BasicResource> loadResource(int game_id, int resource_index);
    std::vector<std::shared_ptr<BasicResource>> loadTileResources(int game_id);
    bool updateResource(int game_id, const std::shared_ptr<BasicResource>& resource);
    bool deleteResource(int game_id, int resource_index);

    std::vector<int> loadTribeTechnologies(int game_id, int tribe_id);
    bool saveTribeTechnology(int game_id, int tribe_id, int technology_id, bool is_known);
    bool updateTribeTechnology(int game_id, int tribe_id, int technology_id, bool is_known);

    int saveAchievement(int game_id, int achievement_id, int tribe_id, const std::string& achive_type, int progress, bool completed);
    std::vector<int> loadTribeAchievements(int game_id, int tribe_id);
    bool updateAchievementProgress(int game_id, int achievement_id, int progress, bool completed);

    bool saveTribeAbility(int game_id, int tribe_id, const std::string& ability_type);
    std::vector<std::string> loadTribeAbilities(int game_id, int tribe_id);
    bool removeTribeAbility(int game_id, int tribe_id, const std::string& ability_type);

    bool saveTribeAvailableBuilding(int game_id, int tribe_id, const std::string& building_type);
    bool saveTribeAvailableUnit(int game_id, int tribe_id, const std::string& unit_type);
    bool saveTribeAvailableResource(int game_id, int tribe_id, const std::string& resource_type);
    bool saveTribeAvailableAchievement(int game_id, int tribe_id, const std::string& achive_type);
    bool saveTribeAvailableDefence(int game_id, int tribe_id, const std::string& defence_type);

    std::vector<std::string> loadTribeAvailableBuildings(int game_id, int tribe_id);
    std::vector<std::string> loadTribeAvailableUnits(int game_id, int tribe_id);
    std::vector<std::string> loadTribeAvailableResources(int game_id, int tribe_id);
    std::vector<std::string> loadTribeAvailableAchievements(int game_id, int tribe_id);
    std::vector<std::string> loadTribeAvailableDefences(int game_id, int tribe_id);

    int saveCityImprovement(int game_id, int tribe_id, int city_id, const std::string& imp_type);
    std::vector<int> loadCityImprovements(int game_id, int tribe_id, int city_index);
    bool deleteCityImprovement(int game_id, int imp_id);

    int getLastGameId() const noexcept;
    size_t getTotalGames() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    int size(){return last_game_id_;};

    std::optional<Player> savePlayer(const Player& player);
    std::optional<Player> loadPlayerByNick(const std::string& nick);
    bool saveUserSession(int user_id, const std::string& token_hash, int expires_in_hours = 24);
    std::optional<int> validateUserToken(const std::string& token_hash);
    bool deactivateUserSession(const std::string& token_hash);

    std::vector<GameInfo> getAvailableGames();
    std::optional<GameInfo> getGameInfo(int game_id);
    bool updateGameStatus(int game_id, const std::string& status);
    std::optional<Player> loadPlayerById(int user_id);
    bool updatePlayerTribe(int game_id, int user_id, int tribe_id);
    [[nodiscard]] int getCityId(int game_id, int x, int y);
    bool updateTribeCapital(int game_id, int tribe_id, int capital_city_id);
private:
    struct PgConnDeleter {
        void operator()(pg_conn* conn) const noexcept;
    };

    std::unique_ptr<pg_conn, PgConnDeleter> conn_;
    std::unordered_map<int, std::shared_ptr<GameSession>> cache_;
    std::unordered_map<std::string, int> tile_id_cache_;

    mutable std::mutex mutex_;
    mutable std::mutex transaction_mutex_;
    std::atomic<int> last_game_id_;
    std::atomic<bool> in_transaction_;

    bool execute(const std::string& query) const;
    std::shared_ptr<pg_result> fetchQuery(const std::string& query) const;
    std::string escapeString(const std::string& str) const;
    void clearCache();
    void invalidateCache(int game_id);
    void logError(const std::string& context, const std::string& error) const;
    int getTileId(int game_id, int x, int y);

};
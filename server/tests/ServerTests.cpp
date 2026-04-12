//
// Created by sigris on 11.04.2026.
//

#include <gtest/gtest.h>
#include <libpq-fe.h>
#include "Repository/GameRepository.h"
#include "EngineElements/IndexDisposer.h"
#include "HttpTestClient.h"
#include "EngineElements/InfoEstimator.h"

const int GAME_ID = 3;
const int TRIBE_1 = 1;
const int TRIBE_2 = 2;
const uint16_t PORT = 18080;
const std::string TOKEN_1 = "50def160a66e6a13eb07c4afb9f0bbfa6379e56ca29e0c4109881ec3467349bd";

const int CAP_1_X = 6, CAP_1_Y = 8;
const int CAP_2_X = 4, CAP_2_Y = 15;
const int MOVE_VALID_X = 5, MOVE_VALID_Y = 8;
const int MOUNTAIN_X = 6, MOUNTAIN_Y = 7;
const int LUMBER_X = 8, LUMBER_Y = 7;

const int FORESTRY_X = 2, FORESTRY_Y = 8;

bool sendAction(int acting, int mainAction, int confirmAction,
                int fromX, int fromY, int toX, int toY,
                const std::string& token = "") {

    nlohmann::json payload = {
            {"game_id", GAME_ID},
            {"acting", acting},
            {"mainAction", mainAction},
            {"confirmAction", confirmAction},
            {"fromX", fromX},
            {"fromY", fromY},
            {"toX", toX},
            {"toY", toY}
    };

    auto response = send_json_request("127.0.0.1", PORT, "/api/game/action", payload, token);
    return response && response->value("status", "") == "ok";
}

class GameplayTest : public ::testing::Test {
protected:
    GameRepository repo;

    void SetUp() override {
        ASSERT_TRUE(repo.connect("host=127.0.0.1 port=5435 dbname=miniciv_test user=sigris password=123456 connect_timeout=3"));
    }
};

TEST_F(GameplayTest, Hire_Warrior_OwnCapital_Positive) {

    bool hired = sendAction(3, 8, 0, CAP_1_X, CAP_1_Y, 1, 0, TOKEN_1);
    EXPECT_TRUE(hired);

    auto units = repo.loadTribeUnits(GAME_ID, TRIBE_1);
    bool found = std::any_of(units.begin(), units.end(),
                             [](const auto& u) { return u && u->x == CAP_1_X && u->y == CAP_1_Y && u->attackType == UnitAttackType::Melee; });
    EXPECT_TRUE(found);
    repo.deleteUnit(GAME_ID, TRIBE_1, CAP_1_X, CAP_1_Y);
}

TEST_F(GameplayTest, Hire_Archer_OwnCapital_Positive) {

    bool hired = sendAction(3, 8, 0, CAP_1_X, CAP_1_Y, 3, 0, TOKEN_1);
    EXPECT_TRUE(hired);

    auto units = repo.loadTribeUnits(GAME_ID, TRIBE_1);
    bool found = std::any_of(units.begin(), units.end(),
                             [](const auto& u) { return u && u->x == CAP_1_X && u->y == CAP_1_Y && u->attackType == UnitAttackType::Ranged; });
    EXPECT_TRUE(found);
    repo.deleteUnit(GAME_ID, TRIBE_1, CAP_1_X, CAP_1_Y);
}

TEST_F(GameplayTest, Hire_EnemyCapital_Negative) {

    bool hired = sendAction(3, 8, 0, CAP_2_X, CAP_2_Y, 1, 0, TOKEN_1);
    EXPECT_FALSE(hired);

    auto enemy_units = repo.loadTribeUnits(GAME_ID, TRIBE_2);
    bool intruder = std::any_of(enemy_units.begin(), enemy_units.end(),
                                [](const auto& u) { return u && u->tribeId == TRIBE_1 && u->x == CAP_2_X && u->y == CAP_2_Y; });
    EXPECT_FALSE(intruder);
}

TEST_F(GameplayTest, Hire_And_Move_Valid_Positive) {
    ASSERT_TRUE(sendAction(3, 8, 0, CAP_1_X, CAP_1_Y, 1, 0, TOKEN_1));

    bool moved = sendAction(0, 0, 2, CAP_1_X, CAP_1_Y, MOVE_VALID_X, MOVE_VALID_Y, TOKEN_1);
    EXPECT_TRUE(moved);

    auto unit = repo.loadUnit(GAME_ID, TRIBE_1, MOVE_VALID_X, MOVE_VALID_Y);
    ASSERT_NE(unit, nullptr);
    EXPECT_EQ(unit->x, MOVE_VALID_X);
    EXPECT_EQ(unit->y, MOVE_VALID_Y);
    repo.deleteUnit(GAME_ID, TRIBE_1, MOVE_VALID_X, MOVE_VALID_Y);
}

TEST_F(GameplayTest, Move_To_Mountains_Negative) {
    ASSERT_TRUE(sendAction(3, 8, 0, CAP_1_X, CAP_1_Y, 1, 0, TOKEN_1));

    auto units = repo.loadTribeUnits(GAME_ID, TRIBE_1);
    ASSERT_FALSE(units.empty());
    auto u = units[0];
    int old_x = u->x, old_y = u->y;

    bool moved = sendAction(0, 0, 2, CAP_1_X, CAP_1_Y, MOUNTAIN_X, MOUNTAIN_Y, TOKEN_1);
    EXPECT_FALSE(moved) << "Move to mountains should be rejected";

    auto still = repo.loadUnit(GAME_ID, TRIBE_1, old_x, old_y);
    ASSERT_NE(still, nullptr);
    EXPECT_EQ(still->x, old_x);
    EXPECT_EQ(still->y, old_y);
    repo.deleteUnit(GAME_ID, TRIBE_1, CAP_1_X, CAP_1_Y);
}

TEST_F(GameplayTest, Study_Climbing_Then_Move_Mountains_Positive) {
    ASSERT_TRUE(sendAction(3, 8, 0, CAP_1_X, CAP_1_Y, 1, 0, TOKEN_1));

    bool studied = sendAction(1, 3, 0, 0, 0, 1, 2, TOKEN_1);
    ASSERT_TRUE(studied);

    auto techs = repo.loadTribeTechnologies(GAME_ID, TRIBE_1);

    auto units = repo.loadTribeUnits(GAME_ID, TRIBE_1);
    ASSERT_FALSE(units.empty());
    auto u = units[0];

    bool moved = sendAction(0, 0, 2, CAP_1_X, CAP_1_Y, MOUNTAIN_X, MOUNTAIN_Y, TOKEN_1);
    EXPECT_TRUE(moved);

    auto updated = repo.loadUnit(GAME_ID, TRIBE_1, MOUNTAIN_X, MOUNTAIN_Y);
    ASSERT_NE(updated, nullptr);
    EXPECT_EQ(updated->x, MOUNTAIN_X);
    EXPECT_EQ(updated->y, MOUNTAIN_Y);
    repo.deleteUnit(GAME_ID, TRIBE_1, MOUNTAIN_X, MOUNTAIN_Y);
    std::string deleteMountain = "DELETE FROM tribe_technologies WHERE technology_id = 2;";
    repo.execute(deleteMountain);
}

TEST_F(GameplayTest, Build_LumberHat_No_Forestry_Negative) {

    auto tile = repo.loadTile(GAME_ID, LUMBER_X, LUMBER_Y);
    std::ostringstream q;
    q << "SELECT resource_type FROM resources WHERE game_id=" << GAME_ID
      << " AND tile_id=" << 5368;

    auto res = repo.fetchQuery(q.str());
    if (res) {
        for (int i = 0; i < PQntuples(res.get()); ++i) {
            std::string res_name = PQgetvalue(res.get(), i, 0);
            ResourceType type = IndexDisposer::getResourceTypeByName(res_name);

            auto resource = InfoEstimator::estimateResource(type);
            if (resource) {
                tile->resources.push_back(resource);
            }
        }
    }
    ASSERT_NE(tile, nullptr);
    bool has_forest = std::any_of(tile->resources.begin(), tile->resources.end(),
                                  [](const auto& r) { return r && r->getType() == ResourceType::Forest; });
    ASSERT_TRUE(has_forest);

    bool built = sendAction(2, 7, 0, LUMBER_X, LUMBER_Y, 6, 0, TOKEN_1);
    EXPECT_FALSE(built);

    int tile_id = repo.getTileId(GAME_ID, LUMBER_X, LUMBER_Y);
    auto buildings = repo.loadTileBuildings(GAME_ID, tile_id, tile->type);
    bool lumber_exists = std::any_of(buildings.begin(), buildings.end(),
                                     [](const auto& b) { return b && b->type == BuildingType::LumberHat; });
    EXPECT_FALSE(lumber_exists);
}

TEST_F(GameplayTest, Study_Hunting_Collect_Animal_Positive) {

    bool studied = sendAction(1, 3, 0, 0, 0, 1, 4, TOKEN_1);
    ASSERT_TRUE(studied);

    auto tile = repo.loadTile(GAME_ID, LUMBER_X, LUMBER_Y);
    std::ostringstream q;
    q << "SELECT resource_type FROM resources WHERE game_id=" << GAME_ID
      << " AND tile_id=" << 5368;

    auto res = repo.fetchQuery(q.str());
    if (res) {
        for (int i = 0; i < PQntuples(res.get()); ++i) {
            std::string res_name = PQgetvalue(res.get(), i, 0);
            ResourceType type = IndexDisposer::getResourceTypeByName(res_name);

            auto resource = InfoEstimator::estimateResource(type);
            if (resource) {
                tile->resources.push_back(resource);
            }
        }
    }
    ASSERT_NE(tile, nullptr);
    int tile_id = repo.getTileId(GAME_ID, LUMBER_X, LUMBER_Y);
    bool has_animal = std::any_of(tile->resources.begin(), tile->resources.end(),
                                  [](const auto& r) { return r && r->getType() == ResourceType::Animal; });
    ASSERT_TRUE(has_animal);

    bool collected = sendAction(2, 6, 0, LUMBER_X, LUMBER_Y, 5, 0, TOKEN_1);
    EXPECT_TRUE(collected);

    auto tile_after = repo.loadTile(GAME_ID, LUMBER_X, LUMBER_Y);
    bool animal_gone = std::none_of(tile_after->resources.begin(), tile_after->resources.end(),
                                    [](const auto& r) { return r && r->getType() == ResourceType::Animal; });
    EXPECT_TRUE(animal_gone);
    std::string deleteAnimal = "DELETE FROM tribe_technologies WHERE technology_id = 4;";
    repo.execute(deleteAnimal);
}
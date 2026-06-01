CREATE TYPE nation_type AS ENUM (
    'Climbers', 'FruitCollectors', 'Hunters', 'Riders', 'Fishermen',
    'Archers', 'Rich', 'Swordsmen', 'Farmers', 'Peacemakers',
    'ShieldBearers', 'RoadCreators'
);

CREATE TYPE terrain_type AS ENUM (
    'None', 'Field', 'Forest', 'Mountain', 'Water', 'DeepWater'
);

CREATE TYPE resource_type AS ENUM (
    'None', 'Forest', 'Fish', 'Mining', 'Farm', 'Animal', 'Fruit'
);

CREATE TYPE building_type AS ENUM (
    'None', 'Market', 'Road', 'Bridge', 'Forge', 'Mill', 'LumberHat',
    'Temple', 'WaterTemple', 'MountainTemple', 'ForestTemple', 'Port',
    'ForestHouse', 'Mining', 'Farming', 'TowerOfWisdom', 'AltarOfPeace',
    'ImperialTomb', 'EyeOfGod', 'FortunePark', 'KillerGates', 'GreatBazaar'
);

CREATE TYPE unit_type AS ENUM (
    'None', 'Warrior', 'Rider', 'Archer', 'Knight', 'Defender',
    'Swordsman', 'Priest', 'Catapult', 'Giant', 'Boat', 'Scout',
    'Ram', 'Squadron', 'Rampager'
);

CREATE TYPE unit_movement_type AS ENUM ('Overland', 'Water', 'Air');

CREATE TYPE unit_attack_type AS ENUM ('Melee', 'Ranged', 'Enticement', 'Magic');

CREATE TYPE achive_type AS ENUM (
    'None', 'Killer', 'Trader', 'Explorer', 'Peace', 'Improve', 'Economic', 'Wisdom'
);

CREATE TYPE defence_type AS ENUM ('None', 'Forest', 'Mountain', 'Water');

CREATE TYPE abilities_type AS ENUM (
    'None', 'Destroying', 'Disband', 'Cutting', 'Growing',
    'Literacy', 'Burning', 'DeepFloating', 'Climbing', 'Floating'
    );

CREATE TABLE games (
                       id SERIAL PRIMARY KEY,
                       map_size INTEGER NOT NULL,
                       created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       is_finished BOOLEAN DEFAULT FALSE,
                       winner_tribe_id INTEGER,
                       current_player INTEGER DEFAULT 0

);

CREATE TABLE tribes (
                        id SERIAL PRIMARY KEY,
                        game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                        tribe_id INTEGER,
                        nation_type nation_type NOT NULL,
                        balance INTEGER DEFAULT 0,
                        capital_city_id INTEGER,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        UNIQUE(game_id, tribe_id)
);

CREATE TABLE tiles (
                       id SERIAL PRIMARY KEY,
                       game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                       x INTEGER NOT NULL,
                       y INTEGER NOT NULL,
                       terrain_type terrain_type NOT NULL DEFAULT 'Field',
                       has_road BOOLEAN DEFAULT FALSE,
                       has_bridge BOOLEAN DEFAULT FALSE,
                       owner_tribe_id INTEGER,
                       defence_modifier NUMERIC(3,2) DEFAULT 1.0,
                       created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       UNIQUE(game_id, x, y),
                       FOREIGN KEY (game_id, owner_tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE SET NULL
);

CREATE TABLE cities (
                        id SERIAL PRIMARY KEY,
                        game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                        tribe_id INTEGER NOT NULL,
                        main_tile_id INTEGER REFERENCES tiles(id) ON DELETE CASCADE,
                        size INTEGER DEFAULT 1,
                        basic_economic INTEGER DEFAULT 0,
                        additional_economic INTEGER DEFAULT 0,
                        current_population INTEGER DEFAULT 0,
                        unit_count INTEGER DEFAULT 0,
                        advanced_territory BOOLEAN DEFAULT FALSE,
                        defence_bonus NUMERIC(3,2) DEFAULT 2.0,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        UNIQUE(game_id, main_tile_id),
                        FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

ALTER TABLE tribes
    ADD CONSTRAINT fk_tribes_capital_city
        FOREIGN KEY (capital_city_id)
            REFERENCES cities(id) ON DELETE SET NULL;

CREATE TABLE units (
                       id SERIAL PRIMARY KEY,
                       game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                       tribe_id INTEGER NOT NULL,
                       tile_id INTEGER REFERENCES tiles(id) ON DELETE SET NULL,
                       unit_type unit_type NOT NULL,
                       x INTEGER NOT NULL,
                       y INTEGER NOT NULL,
                       health INTEGER NOT NULL,
                       damage NUMERIC(4,1) NOT NULL,
                       defence NUMERIC(3,1) NOT NULL,
                       movement INTEGER NOT NULL,
                       attack_range INTEGER NOT NULL,
                       cost INTEGER NOT NULL,
                       movement_type unit_movement_type NOT NULL,
                       attack_type unit_attack_type NOT NULL,
                       kill_counter INTEGER DEFAULT 0,
                       created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       UNIQUE(game_id, tribe_id, x, y),
                       FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE buildings (
                           id SERIAL PRIMARY KEY,
                           game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                           tile_id INTEGER REFERENCES tiles(id) ON DELETE CASCADE,
                           city_id INTEGER REFERENCES cities(id) ON DELETE CASCADE,
                           building_type building_type NOT NULL,
                           cost INTEGER NOT NULL,
                           necessary_terrain terrain_type NOT NULL,
                           is_unique BOOLEAN DEFAULT FALSE,
                           population_bonus INTEGER DEFAULT 0,
                           economic_bonus INTEGER DEFAULT 0,
                           size INTEGER DEFAULT 0,
                           created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                           updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                           UNIQUE(game_id, tile_id, building_type)
);

CREATE TABLE resources (
                           id SERIAL PRIMARY KEY,
                           game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                           tile_id INTEGER REFERENCES tiles(id) ON DELETE CASCADE,
                           resource_type resource_type NOT NULL,
                           quantity INTEGER DEFAULT 1,
                           production_rate INTEGER DEFAULT 1,
                           created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                           updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                           UNIQUE(game_id, tile_id, resource_type)
);

CREATE TABLE technologies (
                              id SERIAL PRIMARY KEY,
                              basic_cost INTEGER NOT NULL,
                              ranged_level INTEGER NOT NULL,
                              new_unit unit_type DEFAULT 'None',
                              new_resource resource_type DEFAULT 'None',
                              new_achive achive_type DEFAULT 'None',
                              new_defence defence_type DEFAULT 'None',
                              new_ability abilities_type DEFAULT 'None',
                              new_building1 building_type DEFAULT 'None',
                              new_building2 building_type DEFAULT 'None',
                              created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                              updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                              name VARCHAR(50) NOT NULL DEFAULT '',
                              tech_x INT NOT NULL DEFAULT 0,
                              tech_y INT NOT NULL DEFAULT 0
);

CREATE TABLE technology_buildings (
                                      technology_id INTEGER NOT NULL REFERENCES technologies(id) ON DELETE CASCADE,
                                      building_type building_type NOT NULL,
                                      PRIMARY KEY (technology_id, building_type)
);

CREATE TABLE tribe_technologies (
                                    id SERIAL PRIMARY KEY,
                                    game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                    tribe_id INTEGER NOT NULL,
                                    technology_id INTEGER NOT NULL REFERENCES technologies(id),
                                    is_known BOOLEAN DEFAULT FALSE,
                                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                    UNIQUE(game_id, tribe_id, technology_id),
                                    FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id)
);

CREATE TABLE achievements (
                              id SERIAL PRIMARY KEY,
                              game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                              achievement_id INTEGER NOT NULL,
                              tribe_id INTEGER NOT NULL,
                              achive_type achive_type NOT NULL,
                              progress INTEGER DEFAULT 0,
                              completed BOOLEAN DEFAULT FALSE,
                              created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                              updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                              UNIQUE(game_id, achievement_id),
                              FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE tribe_abilities (
                                 id SERIAL PRIMARY KEY,
                                 game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                 tribe_id INTEGER NOT NULL,
                                 ability_type abilities_type NOT NULL,
                                 created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                 updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                 UNIQUE(game_id, tribe_id, ability_type),
                                 FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE tribe_available_buildings (
                                           game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                           tribe_id INTEGER NOT NULL,
                                           building_type building_type NOT NULL,
                                           PRIMARY KEY (game_id, tribe_id, building_type),
                                           FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE tribe_available_units (
                                       game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                       tribe_id INTEGER NOT NULL,
                                       unit_type unit_type NOT NULL,
                                       PRIMARY KEY (game_id, tribe_id, unit_type),
                                       FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE tribe_available_resources (
                                           game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                           tribe_id INTEGER NOT NULL,
                                           resource_type resource_type NOT NULL,
                                           PRIMARY KEY (game_id, tribe_id, resource_type),
                                           FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE tribe_available_achievements (
                                              game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                              tribe_id INTEGER NOT NULL,
                                              achive_type achive_type NOT NULL,
                                              PRIMARY KEY (game_id, tribe_id, achive_type),
                                              FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE tribe_available_defences (
                                          game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                          tribe_id INTEGER NOT NULL,
                                          defence_type defence_type NOT NULL,
                                          PRIMARY KEY (game_id, tribe_id, defence_type),
                                          FOREIGN KEY (game_id, tribe_id) REFERENCES tribes(game_id, tribe_id) ON DELETE CASCADE
);

CREATE TABLE city_improvements (
                                   id SERIAL PRIMARY KEY,
                                   game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                                   tribe_id INTEGER NOT NULL,
                                   city_id INTEGER NOT NULL,
                                   improvement_type VARCHAR(50) NOT NULL,
                                   created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                   updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                   UNIQUE(game_id, city_id, improvement_type),
                                   FOREIGN KEY (city_id) REFERENCES cities(id) ON DELETE CASCADE
);

CREATE INDEX idx_tiles_game_coordinates ON tiles(game_id, x, y);
CREATE INDEX idx_tiles_owner ON tiles(game_id, owner_tribe_id);
CREATE INDEX idx_cities_game_tribe ON cities(game_id, tribe_id);
CREATE INDEX idx_cities_main_tile ON cities(main_tile_id);
CREATE INDEX idx_units_game_tribe_pos ON units(game_id, tribe_id, x, y);
CREATE INDEX idx_units_game_tribe ON units(game_id, tribe_id);
CREATE INDEX idx_units_tile ON units(tile_id);
CREATE INDEX idx_buildings_tile ON buildings(tile_id);
CREATE INDEX idx_buildings_city ON buildings(city_id);
CREATE INDEX idx_buildings_game ON buildings(game_id);
CREATE INDEX idx_resources_tile ON resources(tile_id);
CREATE INDEX idx_resources_game ON resources(game_id);
CREATE INDEX idx_tribe_tech ON tribe_technologies(game_id, tribe_id);
CREATE INDEX idx_achievements ON achievements(game_id, tribe_id);
CREATE INDEX idx_tribe_abilities ON tribe_abilities(game_id, tribe_id);
CREATE INDEX idx_tribes_game ON tribes(game_id);

CREATE TABLE users (
                       id SERIAL PRIMARY KEY,
                       nick VARCHAR(50) UNIQUE NOT NULL,
                       password_hash VARCHAR(64) NOT NULL,
                       rating INTEGER DEFAULT 0,
                       created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                       updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE user_sessions (
                               id SERIAL PRIMARY KEY,
                               user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
                               token_hash VARCHAR(64) NOT NULL UNIQUE,
                               created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                               updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                               expires_at TIMESTAMP NOT NULL,
                               is_active BOOLEAN DEFAULT TRUE
);

CREATE INDEX idx_user_sessions_token ON user_sessions(token_hash);
CREATE INDEX idx_user_sessions_user_active ON user_sessions(user_id, is_active, expires_at);

CREATE TABLE game_players (
                              game_id INTEGER NOT NULL REFERENCES games(id) ON DELETE CASCADE,
                              user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
                              tribe_id INTEGER,
                              is_ready BOOLEAN DEFAULT FALSE,
                              joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                              updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                              PRIMARY KEY (game_id, user_id)
);

CREATE INDEX idx_game_players_user ON game_players(user_id);
ALTER TABLE games ADD COLUMN IF NOT EXISTS status VARCHAR(20) DEFAULT 'waiting';

ALTER TABLE games ADD COLUMN IF NOT EXISTS max_players INTEGER DEFAULT 4;
ALTER TABLE games ADD COLUMN IF NOT EXISTS is_private BOOLEAN DEFAULT FALSE;
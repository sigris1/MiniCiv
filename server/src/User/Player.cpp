//
// Created by sigris on 22.03.2026.
//

#include "../include/User/Player.h"

Player::Player(int id, std::string uNick, std::string pass) :
        User(id, std::move(uNick), std::move(pass))
{}

void Player::assertGame(std::shared_ptr<GameSession> game) {
    games_.push_back(game);
    game->confirmPlayer(id_);
}

void Player::makeTurn(std::shared_ptr<GameSession> game){

}

void Player::makeAction(std::shared_ptr<GameSession> game, std::weak_ptr<Action> action){

}

void Player::concedeGame(std::shared_ptr<GameSession> game){
    game->concededByPlayer(id_);
    games_.erase(std::find_if(games_.begin(), games_.end(),
                             [&game](const std::shared_ptr<GameSession>& curGame){
                                game->gameId == curGame->gameId;
                            }));
}

std::shared_ptr<Tribe> Player::chooseTribe(bool isRandom, NationType type){
    if (isRandom) {
        return std::make_shared<Tribe>(id_, RandomGenerator::generateTribeType());
    } else {
        return std::make_shared<Tribe>(id_, type);
    }
}
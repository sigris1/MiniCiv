//
// Created by sigris on 22.03.2026.
//

#include "User/Player.h"
#include "openssl/sha.h"
#include "sstream"
#include "iomanip"

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
                             [&game](const std::weak_ptr<GameSession>& curGame){
                                return game->gameId == curGame.lock()->gameId;
                            }));
}

std::shared_ptr<Tribe> Player::chooseTribe(bool isRandom, NationType type){
    if (isRandom) {
        return std::make_shared<Tribe>(id_, RandomGenerator::generateTribeType());
    } else {
        return std::make_shared<Tribe>(id_, type);
    }
}

std::optional<Player> Player::createNew(const std::string& nick, const std::string& plain_password) {
    if (nick.empty() || nick.length() > 50 || plain_password.length() < 6) {
        return std::nullopt;
    }
    return Player(-1, nick, sha256(plain_password));
}

std::string Player::sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    std::ostringstream oss;
    for (unsigned char c : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return oss.str();
}

bool Player::tryAuthorize(const std::string& plain_password) const {
    return sha256(plain_password) == password_hash_;
}

std::string Player::getHashPassword() const{
    return password_hash_;
}

std::string Player::generateAuthToken() const {
    return generateTokenString(id_);
}

std::string Player::generateTokenString(int user_id) {
    std::ostringstream seed;
    seed << user_id << ":"
         << std::chrono::system_clock::now().time_since_epoch().count()
         << ":miniciv_secret_2026";
    return sha256(seed.str());
}
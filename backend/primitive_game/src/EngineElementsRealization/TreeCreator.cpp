//
// Created by sigris on 02.03.2026.
//

#include <utility>
#include "EngineElements/TreeCreator.h"
#include "Models/Techs/Techs.h"

void buildLine(TechTree& tree, std::shared_ptr<TechNode> start, std::shared_ptr<TechNode> ff, std::shared_ptr<TechNode> fs,
                                                                std::shared_ptr<TechNode> sf, std::shared_ptr<TechNode> ss,
                                                                std::string startN, std::string ffN, std::string fsN,
                                                                                    std::string sfN, std::string ssN){
    tree.root->nextNodes[startN] = std::move(start);
    tree.root->nextNodes[startN]->nextNodes[ffN] = std::move(ff);
    tree.root->nextNodes[startN]->nextNodes[sfN] = std::move(sf);
    tree.root->nextNodes[startN]->nextNodes[ffN]->nextNodes[fsN] = std::move(fs);
    tree.root->nextNodes[startN]->nextNodes[sfN]->nextNodes[ssN] = std::move(ss);
};

void TechTree::create() {
    root = std::make_unique<TechNode>(nullptr);

    // first tier techs
    auto organization = std::make_unique<OrganizationTech>();
    auto orgNode = std::make_shared<TechNode>(std::move(organization));
    auto climbing = std::make_unique<ClimbingTech>();
    auto climbNode = std::make_shared<TechNode>(std::move(climbing));
    auto fishing = std::make_unique<FishingTech>();
    auto fishNode = std::make_shared<TechNode>(std::move(fishing));
    auto hunting = std::make_unique<HuntingTech>();
    auto huntNode = std::make_shared<TechNode>(std::move(hunting));
    auto riding = std::make_unique<RidingTech>();
    auto rideNode = std::make_shared<TechNode>(std::move(riding));

    // second tier techs
    auto farming = std::make_unique<FarmingTech>();
    auto farmNode = std::make_shared<TechNode>(std::move(farming));
    auto shield = std::make_unique<StrategyTech>();
    auto shieldNode = std::make_shared<TechNode>(std::move(shield));
    auto mining = std::make_unique<MiningTech>();
    auto mineNode = std::make_shared<TechNode>(std::move(mining));
    auto meditation = std::make_unique<MeditationTech>();
    auto meditNode = std::make_shared<TechNode>(std::move(meditation));
    auto scout = std::make_unique<ScoutingTech>();
    auto scoutNode =std::make_shared<TechNode>(std::move(scout));
    auto ram = std::make_unique<RammingTech>();
    auto ramNode = std::make_shared<TechNode>(std::move(ram));
    auto archer = std::make_unique<ArcheryTech>();
    auto archerNode = std::make_shared<TechNode>(std::move(archer));
    auto forestry = std::make_unique<ForestryTech>();
    auto forestNode = std::make_shared<TechNode>(std::move(forestry));
    auto roads = std::make_unique<RoadsTech>();
    auto roadNode = std::make_shared<TechNode>(std::move(roads));
    auto freeSpirit = std::make_unique<FreeSpiritTech>();
    auto freeNode = std::make_shared<TechNode>(std::move(freeSpirit));

    //third tier techs
    auto mill = std::make_unique<ConstructionTech>();
    auto millNode = std::make_shared<TechNode>(std::move(mill));
    auto diplomacy = std::make_unique<DiplomacyTech>();
    auto dipNode = std::make_shared<TechNode>(std::move(diplomacy));
    auto smith = std::make_unique<SmitheryTech>();
    auto smithNode = std::make_shared<TechNode>(std::move(smith));
    auto philosophy = std::make_unique<PhilosophyTech>();
    auto philoNode = std::make_shared<TechNode>(std::move(philosophy));
    auto squadron = std::make_unique<NavigationTech>();
    auto squadNode = std::make_shared<TechNode>(std::move(squadron));
    auto aquatism = std::make_unique<AquatismTech>();
    auto aquaNode = std::make_shared<TechNode>(std::move(aquatism));
    auto spiritualism = std::make_unique<SpiritualismTech>();
    auto spiritNode = std::make_shared<TechNode>(std::move(spiritualism));
    auto mathematics = std::make_unique<MathematicsTech>();
    auto matNode = std::make_shared<TechNode>(std::move(mathematics));
    auto trade = std::make_unique<TradeTech>();
    auto tradeNode = std::make_shared<TechNode>(std::move(trade));
    auto knight = std::make_unique<ChivalryTech>();
    auto knightNode = std::make_shared<TechNode>(std::move(knight));

    buildLine(*this, orgNode, farmNode, millNode, shieldNode, dipNode,
              "organization", "farming", "construction", "strategy", "diplomacy");

    buildLine(*this, climbNode, mineNode, smithNode, meditNode, philoNode,
              "climbing", "mining", "smithery", "meditation", "philosophy");

    buildLine(*this, fishNode, scoutNode, squadNode, ramNode, aquaNode,
              "climbing", "mining", "smithery", "meditation", "philosophy");

    buildLine(*this, huntNode, archerNode, spiritNode, forestNode, matNode,
              "climbing", "mining", "smithery", "meditation", "philosophy");

    buildLine(*this, rideNode, roadNode, tradeNode, freeNode, knightNode,
              "climbing", "mining", "smithery", "meditation", "philosophy");

}

TechNode::TechNode(std::unique_ptr<BasicTech> tech):
    currentTech(std::move(tech))
{}

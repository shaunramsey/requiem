#pragma once
#include "Helper.h"
#include <string>
// #include <vector>
#include <imgui.h>
/*
# ID
# card name
# base color
# texture
# attack
# defense
# speed
# gameplay text
# descriptive text
*/

class Card
{
public:
    int id;
    std::string name, description, gameplayText;
    ImVec4 baseColor;
    std::string texturePath;
    int attack, defense, speed;
    Card(int id, const std::string &name,
         const ImVec4 &baseColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
         const std::string &texturePath = "",
         int attack = 0, int defense = 0, int speed = 100,
         const std::string &description = "Undefined",
         const std::string &gameplayText = "")
        : id(id), name(name), description(description), baseColor(baseColor),
          texturePath(texturePath), attack(attack), defense(defense), speed(speed),
          gameplayText(gameplayText) {}

    int getId() const { return id; }
    const std::string &getName() const { return name; }
    const std::string &getDescription() const { return description; }
    std::string toString() const
    {
        return "Card{id: " + std::to_string(id) + ", name: '" + name +
               "', description: '" + description + "', baseColor: " +
               std::to_string(baseColor.x) + ", " + std::to_string(baseColor.y) + ", " +
               std::to_string(baseColor.z) + ", " + std::to_string(baseColor.w) +
               ", texturePath: '" + texturePath + "', attack: " + std::to_string(attack) +
               ", defense: " + std::to_string(defense) +
               ", speed: " + std::to_string(speed) +
               ", gameplayText: '" + gameplayText + "'}";
    }
};

void loadCardsFromToml(const toml::table &tbl, std::vector<Card> &cardList)
{
    _console.Log("TOML", "Loading cards.");
    cardList.clear();
    if (tbl.contains("DreamCards"))
    {
        // std::cout << "DreamCards found in TOML file." << std::endl;

        auto dc = tbl["DreamCards"];
        if (!dc.is_array())
        {
            _console.ErrorLog("TOML", "DreamCards was illformed - aborting card loading!!");
            return;
        }
        auto dreamCards = tbl["DreamCards"].as_array();
        ImVec4 defaultColor(1.0f, 0.0f, 1.0f, 1.0f);
        for (auto &&card : *dreamCards)
        {
            if (card.is_array())
            {
                const toml::array &arr = *card.as_array();
                if (arr.size() >= 9)
                {
                    int id = arr[0].value_or(0);
                    std::string name = arr[1].value_or("Unnamed Card");
                    const toml::array &colorArray = *arr[2].as_array();
                    ImVec4 baseColor = Helper::tomlArrayToImVec4(colorArray, defaultColor);
                    std::string texturePath = arr[3].value_or("");
                    int attack = arr[4].value_or(0);
                    int defense = arr[5].value_or(0);
                    int speed = arr[6].value_or(100);
                    std::string description = arr[7].value_or("No description");
                    std::string gameplayText = arr[8].value_or("No gameplay text");

                    cardList.emplace_back(id, name, baseColor, texturePath, attack, defense, speed, description, gameplayText);
                }
            }
        }
    }
    _console.Log("TOML", "(*) Loaded %d cards from TOML.", cardList.size());
    for (int i = 0; i < cardList.size(); i++)
    {
        _console.DebugLog("TOML", "    [-] %s", cardList[i].toString().c_str());
    }
}
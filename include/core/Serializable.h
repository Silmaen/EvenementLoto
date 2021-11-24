/**
 * @file Serializable.h
 * @author Silmaen
 * @date 17/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "baseDefine.h"
#include <iostream>
#include <nlohmann/json.hpp>

namespace evl::core {

using json= nlohmann::json;
/**
 * @brief Class de base pour la sérialisation dans des fichiers
 */
class Serializable {
public:
    /**
     * @brief Destructeur
     */
    virtual ~Serializable()= default;

    /**
     * @brief Lecture depuis un stream
     * @param bs Le stream d’entrée.
     * @param file_version La version du fichier à lire
     */
    virtual void read(std::istream& bs, int file_version)= 0;

    /**
     * @brief Écriture dans un stream.
     * @param bs Le stream où écrire.
     */
    virtual void write(std::ostream& bs) const= 0;

    /**
     * @brief Écriture dans un json.
     * @return Le json à remplir
     */
    virtual json to_json() const= 0;

    /**
     * @brief Lecture depuis un json
     * @param j Le json à lire
     */
    virtual void from_json(const json& j)= 0;
};

}// namespace evl::core

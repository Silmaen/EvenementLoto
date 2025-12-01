/**
 * @file Serializable.h
 * @author Silmaen
 * @date 17/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "../baseDefine.h"
#include <json/json.h>

namespace evl::core {

/**
 * @brief Class de base pour la sérialisation dans des fichiers
 */
class Serializable {
public:
    Serializable()                              = default;
    Serializable(const Serializable&)           = default;
    Serializable(Serializable&&)                = default;
    Serializable& operator=(const Serializable&)= default;
    Serializable& operator=(Serializable&&)     = default;
    /**
     * @brief Destructeur
     */
    virtual ~Serializable();

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
    virtual Json::Value to_json() const= 0;

    /**
     * @brief Lecture depuis un json
     * @param j Le json à lire
     */
    virtual void from_json(const Json::Value& j)= 0;
};

}// namespace evl::core

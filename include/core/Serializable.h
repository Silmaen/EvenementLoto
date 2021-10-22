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

namespace evl::core {

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
     */
    virtual void read(std::istream& bs)= 0;

    /**
     * @brief Écriture dans un stream.
     * @param bs Le stream où écrire.
     */
    virtual void write(std::ostream& bs) const= 0;
};

}// namespace evl::core

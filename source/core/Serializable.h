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
#include <yaml-cpp/yaml.h>

namespace evl::core {

/**
 * @brief Class de base pour la sérialisation dans des fichiers
 */
class Serializable {
public:
	Serializable() = default;
	Serializable(const Serializable&) = default;
	Serializable(Serializable&&) = default;
	auto operator=(const Serializable&) -> Serializable& = default;
	auto operator=(Serializable&&) -> Serializable& = default;
	/**
	 * @brief Destructeur
	 */
	virtual ~Serializable();

	/**
	 * @brief Lecture depuis un stream
	 * @param iBs Le stream d’entrée.
	 * @param iFileVersion La version du fichier à lire
	 */
	virtual void read(std::istream& iBs, int iFileVersion) = 0;

	/**
	 * @brief Écriture dans un stream.
	 * @param iBs Le stream où écrire.
	 */
	virtual void write(std::ostream& iBs) const = 0;

	/**
	 * @brief Écriture dans un json.
	 * @return Le json à remplir
	 */
	[[nodiscard]] virtual auto toJson() const -> Json::Value = 0;

	/**
	 * @brief Lecture depuis un json
	 * @param iJson Le json à lire
	 */
	virtual void fromJson(const Json::Value& iJson) = 0;

	/**
	 * @brief Écriture dans un YAML node.
	 * @return Le YAML node à remplir
	 */
	[[nodiscard]] virtual auto toYaml() const -> YAML::Node = 0;

	/**
	 * @brief Lecture depuis un YAML node
	 * @param iNode Le YAML node à lire
	 */
	virtual void fromYaml(const YAML::Node& iNode) = 0;
};

}// namespace evl::core

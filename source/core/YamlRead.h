/**
 * @file YamlRead.h
 * @author Silmaen
 * @date 28/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <yaml-cpp/yaml.h>

namespace evl::core {

/**
 * @brief Si une clé est présente, sans rien lever.
 *
 * `IsDefined()` ne répond pas à cette question : il est **vrai** pour un nœud nul, donc
 * vrai pour la valeur d'une clé écrite `clé: ~`. Seul un nœud *invalide* — celui que
 * rend l'indexation d'une clé absente — le rend faux, et c'est la seule question qu'un
 * tel nœud accepte sans lever.
 *
 * @param[in] iNode Le nœud parent.
 * @param[in] iKey La clé cherchée.
 * @return True si la clé est là, quelle que soit sa valeur.
 */
[[nodiscard]] inline auto yamlHas(const YAML::Node& iNode, const char* iKey) -> bool {
	if (!iNode.IsDefined() || !iNode.IsMap())
		return false;
	return iNode[iKey].IsDefined();
}

/**
 * @brief Un nœud enfant, ou un nœud nul si la clé n'est pas là.
 *
 * Indexer un `YAML::Node` constant sur une clé absente rend un nœud *invalide*, et
 * presque tout ce qu'on lui demande ensuite — `IsSequence()`, `IsMap()`, `size()` —
 * lève `YAML::InvalidNode`. Un garde-fou écrit naïvement est donc lui-même le fautif :
 * c'est exactement ainsi qu'une exception remontait jusqu'à la boucle de rendu à
 * l'import d'un fichier qui n'était pas un événement.
 *
 * Un nœud nul plutôt qu'un nœud invalide : tout ce qu'on lui demande ensuite —
 * `IsSequence()`, `as<T>(defaut)` — répond sagement non. Pour savoir si la clé était
 * là, c'est `yamlHas` qu'il faut demander, un nœud nul étant indiscernable d'une clé
 * absente une fois rendu ici.
 *
 * @param[in] iNode Le nœud parent.
 * @param[in] iKey La clé cherchée.
 * @return Le nœud enfant, ou un nœud nul.
 */
[[nodiscard]] inline auto yamlChild(const YAML::Node& iNode, const char* iKey) -> YAML::Node {
	if (!yamlHas(iNode, iKey))
		return {};
	return iNode[iKey];
}

/**
 * @brief La valeur d'une clé, ou le défaut si elle n'est pas là.
 *
 * `yamlChild(...).as<T>(defaut)` ne suffit pas : un nœud nul se convertit sans se
 * plaindre, et `as<std::string>` en tire la chaine **"null"** au lieu du défaut. Un lot
 * dont la désignation manquait s'appelait donc « null ». La présence est donc vérifiée
 * avant toute conversion, et une clé écrite sans valeur — `clé: ~` — vaut une clé
 * absente.
 *
 * @tparam ValueType Le type attendu.
 * @param[in] iNode Le nœud parent.
 * @param[in] iKey La clé cherchée.
 * @param[in] iDefault La valeur rendue si la clé est absente ou illisible.
 * @return La valeur lue, ou le défaut.
 */
template<typename ValueType>
[[nodiscard]] auto yamlValue(const YAML::Node& iNode, const char* iKey, const ValueType& iDefault) -> ValueType {
	if (!yamlHas(iNode, iKey))
		return iDefault;
	const auto child = iNode[iKey];
	if (child.IsNull())
		return iDefault;
	return child.as<ValueType>(iDefault);
}

}// namespace evl::core

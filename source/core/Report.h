/**
 * @file Report.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <string>

namespace evl::core {

class Event;

/**
 * @brief Construit le rapport de fin d'événement, en Markdown.
 *
 * Ce que l'organisateur a besoin de retrouver le lendemain : ce qui s'est joué, qui a
 * gagné quoi, ce que ça valait, et à qui dire merci. Le Markdown est produit ici, dans
 * le cœur, plutôt que dessiné dans l'interface : c'est exactement ce qui s'affiche et
 * ce qui s'enregistre, il n'y a donc pas deux versions du même rapport à tenir à jour.
 *
 * Les parties improvisées y figurent comme les autres, les pauses n'y figurent pas.
 *
 * @param iEvent L'événement à raconter.
 * @return Le rapport, en Markdown.
 */
[[nodiscard]] auto buildReport(const Event& iEvent) -> std::string;

}// namespace evl::core

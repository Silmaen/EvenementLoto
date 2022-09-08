/**
 * @file WinnerInput.h
 * @author damien.lachouette 
 * @date 14/09/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <QDialog>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class WinnerInput;

}// namespace Ui

namespace evl::gui {

/**
 * @brief Class WinnerInput
 */
class WinnerInput: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Default constructor.
     */
    explicit WinnerInput(QWidget* parent= nullptr);
    /**
     * @brief Destructor.
     */
    ~WinnerInput() override;

    /**
     * @brief Renvoie le nom renseigné du gagnant.
     * @return Le nom du gagnant.
     */
    QString getWinner() const;

private:
    /// Lien vers la page UI
    Ui::WinnerInput* ui;
};

}// namespace evl::gui

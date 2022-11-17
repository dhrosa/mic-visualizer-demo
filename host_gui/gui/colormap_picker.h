#pragma once

#include <QComboBox>

class ColormapPicker : public QComboBox {
public:
  ColormapPicker();

private:
  void showPopup() override;
  void hidePopup() override;

  QString previous_text_;
};

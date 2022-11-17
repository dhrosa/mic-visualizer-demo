#pragma once

#include <QComboBox>
#include <string_view>

class ColormapPicker : public QComboBox {
public:
  ColormapPicker(std::string_view original_name);
};

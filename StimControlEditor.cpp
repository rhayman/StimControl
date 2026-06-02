#include "StimControlEditor.hpp"
#include "../../../plugin-GUI/Source/Utils/Utils.h"
#include <iostream>
#include <stdio.h>

StimControlEditor::StimControlEditor(GenericProcessor *proc)
    : GenericEditor(proc) {
  desiredWidth = 520;

  addComboBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "device", 10, 20);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Trigger", 10, 50);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Gate", 10, 80);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Output", 10, 110);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Start", 180, 20);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Stop", 180, 50);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Duration", 180, 80);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Interval", 180, 110);
  addToggleParameterEditor(Parameter::PROCESSOR_SCOPE, "Apply", 360, 110);
}

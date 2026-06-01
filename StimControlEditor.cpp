#include "StimControlEditor.hpp"
#include "../../../plugin-GUI/Source/Utils/Utils.h"
#include <iostream>
#include <stdio.h>

StimControlEditor::StimControlEditor(GenericProcessor *proc)
    : GenericEditor(proc) {
  desiredWidth = 420;

  addComboBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "device", 10, 20);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Trigger", 100, 20);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Gate", 190, 20);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Output", 280, 20);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Start", 10, 60);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Stop", 100, 60);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Duration", 190, 60);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Interval", 280, 60);
  addToggleParameterEditor(Parameter::PROCESSOR_SCOPE, "Apply", 380, 80);
}

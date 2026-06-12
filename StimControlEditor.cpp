#include "StimControlEditor.hpp"

StimControlEditor::StimControlEditor(GenericProcessor *proc)
    : GenericEditor(proc) {
  desiredWidth = 500;

  addComboBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "device", 10, 30);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Trigger", 10, 55);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Gate", 10, 80);
  addTtlLineParameterEditor(Parameter::STREAM_SCOPE, "Output", 10, 105);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Start", 180, 30);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Stop", 180, 55);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Duration", 180, 80);
  addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Interval", 180, 105);
  addComboBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "Protocol", 360, 30);
  addToggleParameterEditor(Parameter::PROCESSOR_SCOPE, "Apply", 360, 105);
}

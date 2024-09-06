#include "StimControlEditor.hpp"
#include "../../../plugin-GUI/Source/Utils/Utils.h"
#include <stdio.h>

StimControlEditor::StimControlEditor(GenericProcessor * proc)
	: GenericEditor(proc)
{
	desiredWidth = 420;

	addComboBoxParameterEditor("Device", 10, 20);
	addComboBoxParameterEditor("Trigger", 100, 20);
	addComboBoxParameterEditor("Gate", 190, 20);
	addComboBoxParameterEditor("Output", 280, 20);
	addTextBoxParameterEditor("Start", 10, 60);
	addTextBoxParameterEditor("Stop", 100, 60);
	addTextBoxParameterEditor("Duration", 190, 60);
	addTextBoxParameterEditor("Interval", 280, 60);
	addCheckBoxParameterEditor("Apply", 380, 80);
}
#include "StimControlEditor.hpp"
#include <stdio.h>

StimControlEditor::StimControlEditor(GenericProcessor * proc, bool useDefaultParameterEditors=true)
	: GenericEditor(proc, useDefaultParameterEditors)
{
	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
	MemoryInputStream mis(silk, silksize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	Font font = Font(typeface);

	desiredWidth = 420;

	m_proc = (StimControl*)proc;
	std::vector<ofSerialDeviceInfo> devices = m_proc->getDeviceList();
	devicePicker = new ComboBox();
	devicePicker->setBounds(10,30,100,20);
	devicePicker->addListener(this);
	devicePicker->setTextWhenNothingSelected (("Device"));
	devicePicker->setTextWhenNoChoicesAvailable (("Device"));

	for (int i = 0; i < devices.size(); ++i)
		devicePicker->addItem(devices[i].getDevicePath(),i+1);

	addAndMakeVisible(devicePicker);

	inputChannelPicker = new ComboBox();
	inputChannelPicker->setBounds(10,55,100,20);
	inputChannelPicker->addListener(this);
	inputChannelPicker->setTextWhenNothingSelected (("Trigger"));
	inputChannelPicker->setTextWhenNoChoicesAvailable (("Trigger"));

	for (int i = 0; i < 13; ++i)
		inputChannelPicker->addItem(String(i+1),i+1);

	addAndMakeVisible(inputChannelPicker);

	gateChannelChannelPicker = new ComboBox();
	gateChannelChannelPicker->setBounds(10,80,100,20);
	gateChannelChannelPicker->addListener(this);
	gateChannelChannelPicker->setTextWhenNothingSelected (("Gate"));
	gateChannelChannelPicker->setTextWhenNoChoicesAvailable (("Gate"));

	for (int i = 0; i < 16; ++i)
		gateChannelChannelPicker->addItem(String(i+1),i+1);

	addAndMakeVisible(gateChannelChannelPicker);

	outputChannelPicker = new ComboBox();
	outputChannelPicker->setBounds(10,105,100,20);
	outputChannelPicker->addListener(this);
	outputChannelPicker->setTextWhenNothingSelected (("Output"));
	outputChannelPicker->setTextWhenNoChoicesAvailable (("Output"));

	for (int i = 0; i < 13; ++i)
		outputChannelPicker->addItem(String(i+1),i+1);

	addAndMakeVisible(outputChannelPicker);

	font.setHeight(11);

	startTimeEditor = new TextEditor(String("StartTime"));
	startTimeEditor->setBounds(120, 30, 55, 20);
	startTimeEditor->setText(String("0"));
	addAndMakeVisible(startTimeEditor);

	startLabel = new Label("StartTime", "Start (s)");
	startLabel->setBounds(180, 30, 80, 20);
	startLabel->setFont(font);
	startLabel->setEditable (false, false, false);
	startLabel->setJustificationType(Justification::centredLeft);
	startLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(startLabel);

	stopTimeEditor = new TextEditor(String("StopTime"));
	stopTimeEditor->setBounds(120, 55, 55, 20);
	stopTimeEditor->setText(String("0"));
	addAndMakeVisible(stopTimeEditor);

	stopLabel = new Label("StopTime", "Stop (s)");
	stopLabel->setBounds(180, 55, 80, 20);
	stopLabel->setFont(font);
	stopLabel->setEditable (false, false, false);
	stopLabel->setJustificationType(Justification::centredLeft);
	stopLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(stopLabel);

	stimOnTime = new TextEditor(String("StimOnDuration"));
	stimOnTime->setBounds(120, 80, 55, 20);
	stimOnTime->setText(String("10"));
	addAndMakeVisible(stimOnTime);

	stimOnTimeLabel = new Label("stimOnDuration", "Stim on (ms)");
	stimOnTimeLabel->setBounds(180, 80, 80, 20);
	stimOnTimeLabel->setFont(font);
	stimOnTimeLabel->setEditable (false, false, false);
	stimOnTimeLabel->setJustificationType(Justification::centredLeft);
	stimOnTimeLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(stimOnTimeLabel);

	stimOffTime = new TextEditor(String("StimOffDuration"));
	stimOffTime->setBounds(120, 105, 55, 20);
	stimOffTime->setText(String("150"));
	addAndMakeVisible(stimOffTime);

	stimOffTimeLabel = new Label("stimOffDuration", "Stim off (ms)");
	stimOffTimeLabel->setBounds(180, 105, 80, 20);
	stimOffTimeLabel->setFont(font);
	stimOffTimeLabel->setEditable (false, false, false);
	stimOffTimeLabel->setJustificationType(Justification::centredLeft);
	stimOffTimeLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(stimOffTimeLabel);

	applyStimButton = new ToggleButton("Apply when recording");
	applyStimButton->setBounds(265, 105, 55, 20); //  smaller than this and the JUCE graphics_context goes mental
	applyStimButton->setToggleState(true, dontSendNotification);
	applyStimButton->addListener(this);
	addAndMakeVisible(applyStimButton);


}

StimControlEditor::~StimControlEditor() {}

void StimControlEditor::receivedEvent() {}

void StimControlEditor::comboBoxChanged(ComboBox * box) {
	if ( box == devicePicker ) {
		deviceId = devicePicker->getSelectedId() - 1;
		deviceStr = devicePicker->getText().toStdString();
		m_proc->setDeviceString(deviceStr);
	}
	if ( box == inputChannelPicker ) {
		inputPin = inputChannelPicker->getSelectedId();
	}
	if ( box == outputChannelPicker ) {
		outputPin = outputChannelPicker->getSelectedId();
	}
	if ( box == gateChannelChannelPicker ) {
		gatePin = gateChannelChannelPicker->getSelectedId();
	}
}

void StimControlEditor::buttonEvent(Button * button) {
}

int StimControlEditor::getStartTime() {
	String time = startTimeEditor->getText();
	return time.getIntValue();
}

int StimControlEditor::getStopTime() {
	String time = stopTimeEditor->getText();
	return time.getIntValue();
}

int StimControlEditor::getStimOnTime() {
	String time = stimOnTime->getText();
	return time.getIntValue();
}

int StimControlEditor::getStimOffTime() {
	String time = stimOffTime->getText();
	return time.getIntValue();
}

int StimControlEditor::getInputPin() {
	return inputPin;
}

int StimControlEditor::getOutputPin() {
	return outputPin;
}

int StimControlEditor::getGatePin() {
	return gatePin;
}

void StimControlEditor::setStartTime(uint16_t val) {
	startTimeEditor->setText(String(static_cast<int>(val)), false);
}

void StimControlEditor::setStopTime(uint16_t val) {
	stopTimeEditor->setText(String(static_cast<int>(val)), false);
}

void StimControlEditor::setStimOnTime(uint16_t val) {
	stimOnTime->setText(String(static_cast<int>(val)), false);
}

void StimControlEditor::setStimOffTime(uint16_t val) {
	stimOffTime->setText(String(static_cast<int>(val)), false);
}

void StimControlEditor::setInputPin(uint16_t val) {
	inputChannelPicker->setSelectedId(static_cast<int>(val));
}

void StimControlEditor::setOutputPin(uint16_t val) {
	outputChannelPicker->setSelectedId(static_cast<int>(val));
}

void StimControlEditor::setDeviceId(std::string device2select) {
	int devices = devicePicker->getNumItems();
	for (int i = 0; i < devices; ++i)
	{
		if ( devicePicker->getItemText(i) == String(device2select) ) {
			devicePicker->setSelectedId(i+1, dontSendNotification);
		}
	}
}
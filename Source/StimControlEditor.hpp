#ifndef STIMCONTROLEDITOR_H_
#define STIMCONTROLEDITOR_H_

// #include <EditorHeaders.h>
#include "StimControl.hpp"

class StimControlEditor : public GenericEditor, public ComboBox::Listener
{
private:
	StimControl * m_proc = nullptr;
	ScopedPointer<ComboBox> inputChannelPicker;
	ScopedPointer<ComboBox> outputChannelPicker;
	ScopedPointer<ComboBox> gateChannelChannelPicker;
	ScopedPointer<ComboBox> devicePicker;
	ScopedPointer<TextEditor> startTimeEditor;
	ScopedPointer<TextEditor> stopTimeEditor;
	ScopedPointer<TextEditor> stimOnTime;
	ScopedPointer<TextEditor> stimOffTime;
	ScopedPointer<ToggleButton> applyStimButton;
	ScopedPointer<Label> startLabel;
	ScopedPointer<Label> stopLabel;
	ScopedPointer<Label> stimOnTimeLabel;
	ScopedPointer<Label> stimOffTimeLabel;
	bool startStop = false;
public:
	StimControlEditor(GenericProcessor *, bool);
	virtual ~StimControlEditor();
	void receivedEvent();
	void comboBoxChanged(ComboBox *);
	void buttonEvent(Button *);
	int getStartTime();
	int getStopTime();
	int getStimOnTime();
	int getStimOffTime();
	int getInputPin();
	int getOutputPin();
	int getGatePin();
	void setStartTime(uint16_t);
	void setStopTime(uint16_t);
	void setStimOnTime(uint16_t);
	void setStimOffTime(uint16_t);
	void setInputPin(uint16_t);
	void setOutputPin(uint16_t);
	void setDeviceId(std::string);
	int deviceId = -1;
	std::string deviceStr = "";
	int inputPin = -1;
	int outputPin = -1;
	int gatePin = -1;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimControlEditor);
};

#endif
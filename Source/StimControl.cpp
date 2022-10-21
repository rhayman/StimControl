#include "StimControl.hpp"
#include "StimControlEditor.hpp"
#include "../../../plugin-GUI/Source/Utils/Utils.h"

#include <stdio.h>

StimControl::StimControl() : GenericProcessor("StimControl"),
	outputChannel(13),
	inputChannel(-1),
	state(true),
	acquisitionActive(false),
	deviceSelected(false)
{
	std::map<std::string, int> devices;
	getDeviceList(devices);
	Array<String> devs;
	for (auto dev : devices)
		devs.add(dev.first);
	addCategoricalParameter(Parameter::GLOBAL_SCOPE, "Device", "Device name", devs, 0);
	addCategoricalParameter(Parameter::GLOBAL_SCOPE, "Trigger", "Trigger line", arduino_lines, 0);
	addCategoricalParameter(Parameter::GLOBAL_SCOPE, "Gate", "Gate line", arduino_lines, 1);
	addCategoricalParameter(Parameter::GLOBAL_SCOPE, "Output", "Output line", arduino_lines, 1);
	addStringParameter(Parameter::GLOBAL_SCOPE, "Start", "Start time", "0");
	addStringParameter(Parameter::GLOBAL_SCOPE, "Stop", "Stop time", "2000");
	addStringParameter(Parameter::GLOBAL_SCOPE, "Duration", "Stimulation duration", "10");
	addStringParameter(Parameter::GLOBAL_SCOPE, "Interval", "Interval duration", "150");
	addBooleanParameter(Parameter::GLOBAL_SCOPE, "Apply", "Apply during recording", true);
	parameterValueChanged(getParameter("Device"));
}

StimControl::~StimControl() {
	sendStringToDevice("<StartRunning, 0, >");
	serial.flush(true, true);
	serial.close();
}

void StimControl::process(AudioSampleBuffer & buffer) {
	checkForEvents();
}

bool StimControl::startAcquisition() {
	if ( ! isDeviceInitialized() ) {
		setupDevice();
	}
	sendData();
	serial.flush(true, true);
	return true;
}

bool StimControl::stopAcquisition() {
	sendStringToDevice("<StartRunning, 0, >");
	serial.flush(true, true);
	return true;
}

void StimControl::startRecording() {
	if ( ! isDeviceInitialized() ) {
		setupDevice();
	}
	sendData();
	serial.flush(true, true);
}

void StimControl::stopRecording() {
	sendStringToDevice("<StartRunning, 0, >");
	serial.flush(true, true);
}

void StimControl::parameterValueChanged(Parameter * param) {
	for (auto stream : getDataStreams()) {
        if (stream->getName().equalsIgnoreCase("StimControl datastream")) {
			auto this_dev = settings[stream->getStreamId()];
			if (param->getName().equalsIgnoreCase("Device")) {
				this_dev->name = ((CategoricalParameter*)(param))->getSelectedString().toStdString();
				std::map<std::string, int> devices;
				getDeviceList(devices);
				this_dev->deviceId = devices[this_dev->name];
				setupDevice();
			}
			else if (param->getName().equalsIgnoreCase("Trigger")) {
				this_dev->inputPin = ((CategoricalParameter*)(param))->getSelectedString().getIntValue();
			}
			else if (param->getName().equalsIgnoreCase("Gate")) {
				// this_dev->name = ((CategoricalParameter*)(param))->getSelectedString().toStdString();
			}
			else if (param->getName().equalsIgnoreCase("Output")) {
				this_dev->outputPin = ((CategoricalParameter*)(param))->getSelectedString().getIntValue();
			}
			else if (param->getName().equalsIgnoreCase("Start")) {
				this_dev->startTime = (int)param->getValue();
			}
			else if (param->getName().equalsIgnoreCase("Stop")) {
				this_dev->stopTime = (int)param->getValue();
			}
			else if (param->getName().equalsIgnoreCase("Duration")) {
				this_dev->stimOnTime = (int)param->getValue();
			}
			else if (param->getName().equalsIgnoreCase("Interval")) {
				this_dev->stimOffTime = (int)param->getValue();
			}	
		}
	}
}

void StimControl::updateSettings() {

	if ( getDataStreams().isEmpty() ) {
        DataStream::Settings streamsettings{"StimControl datastream",
                                            "Datastream for stimulation from Arduino",
                                            "external.stimulation.arduino",
                                            getDefaultSampleRate()};

        auto stream = new DataStream(streamsettings);
        dataStreams.add(stream);
        dataStreams.getLast()->addProcessor(processorInfo.get());

		ContinuousChannel::Settings settings{
            ContinuousChannel::Type::AUX,
            "DUMMY",
            "stim channel",
            "stim.raw",
            1,
            dataStreams.getLast()};
        continuousChannels.add(new ContinuousChannel(settings));
    }
	settings.update(getDataStreams());
	isEnabled = true;
}

AudioProcessorEditor * StimControl::createEditor() {
	editor = std::make_unique<StimControlEditor>(this);
	return editor.get();
}

StimSettings StimControl::getSettings() {
	StimSettings current_settings;
	for (auto stream : getDataStreams()) {
        if (stream->getName().equalsIgnoreCase("StimControl datastream")) {
			auto this_dev = settings[stream->getStreamId()];
			current_settings.hasData = 1;
			current_settings.inputPin = ((CategoricalParameter*)(getParameter("Trigger")))->getSelectedString().getIntValue();
			current_settings.outputPin = ((CategoricalParameter*)(getParameter("Output")))->getSelectedString().getIntValue();
			current_settings.startTime =  (int)getParameter("Start")->getValue();
			current_settings.stopTime =  (int)getParameter("Stop")->getValue();
			current_settings.stimOffTime =  (int)getParameter("Interval")->getValue();
			current_settings.stimOnTime =  (int)getParameter("Duration")->getValue();
		}
	}
	return current_settings;
}

void StimControl::sendData() {
	if (!isDeviceInitialized())
		setupDevice();
	CoreServices::sendStatusMessage("Sending data");
	StimSettings s = getSettings();
	printParams(s);
	sendStringToDevice("<Start," + std::to_string(s.startTime) + ",>");
	sendStringToDevice("<Stop," + std::to_string(s.stopTime) + ",>");
	sendStringToDevice("<OutputPin," + std::to_string(s.outputPin) + ",>");
	sendStringToDevice("<Duration," + std::to_string(s.stimOnTime) + ",>");
	sendStringToDevice("<Interval," + std::to_string(s.stimOffTime) + ",>");
	auto startRunning = (bool)getParameter("Apply")->getValue();
	sendStringToDevice("<StartRunning," + std::to_string(startRunning) + ",>");
	CoreServices::sendStatusMessage("Data sent");
}

int StimControl::sendStringToDevice(std::string const & str) {
	std::vector<unsigned char> cstr(str.data(), str.data() + str.size() + 1);
	return serial.writeBytes(&cstr[0], sizeof(str));
}

void StimControl::deviceInitialized(bool val) {
	isDeviceSetup = val;
}

bool StimControl::isDeviceInitialized() {
	return isDeviceSetup;
}
void StimControl::setupDevice() {
	for (auto stream : getDataStreams()) {
        if (stream->getName().equalsIgnoreCase("StimControl datastream")) {
			auto this_dev = settings[stream->getStreamId()];
			auto selected = this_dev->deviceId;
			if ( selected >= 0 ) {
				CoreServices::sendStatusMessage("Initializing device...");
				serial.setup(selected, baudrate);
				deviceInitialized(true);
				CoreServices::sendStatusMessage("Device successfully initialized");
			}
			else
				CoreServices::sendStatusMessage("Select a device first.");
		}
	}
}

void StimControl::setupDevice(std::string devId) {
	serial.setup(devId, baudrate);
	deviceInitialized(true);
}

std::vector<ofSerialDeviceInfo> StimControl::getDeviceList(){
	return serial.getDeviceList();
}

void StimControl::getDeviceList(std::map<std::string, int>& namesAndIDs) {
	auto devices = getDeviceList();
	for (auto dev : devices) {
		namesAndIDs[dev.getDeviceName()] = dev.getDeviceID();
	}
}

void StimControl::closeDevice() {
	serial.close();
}

void StimControl::printParams(StimSettings settings) {
	LOGC("Has data: ", settings.hasData);
	LOGC("Input pin: ", settings.inputPin);
	LOGC("Output pin: ", settings.outputPin);
	LOGC("Start time (s): ", settings.startTime);
	LOGC("Stop time (s): ", settings.stopTime);
	LOGC("Stim on duration (ms): ", settings.stimOnTime);
	LOGC("Stim off duration (ms): ", settings.stimOffTime);
}
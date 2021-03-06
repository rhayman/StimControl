#include "StimControl.hpp"
#include "StimControlEditor.hpp"

#include <stdio.h>

StimControl::StimControl() : GenericProcessor("StimControl"),
	outputChannel(13),
	inputChannel(-1),
	state(true),
	acquisitionActive(false),
	deviceSelected(false)
{
	setProcessorType(PROCESSOR_TYPE_SINK);
}

StimControl::~StimControl() {
	serial.flush(true, true);
	serial.close();
}

void StimControl::process(AudioSampleBuffer & buffer) {
	checkForEvents();
}

void StimControl::startRecording() {
	if ( ! isDeviceInitialized() ) {
		auto s = getDeviceString();
		setupDevice(s);
	}
	setPinStates();
	setStartAndStopTimes();
	setStimDurations();
	sendData();
	serial.flush(true, true);
	// ofs.open(out_stream_file, std::ofstream::out | std::ofstream::trunc);
}

void StimControl::stopRecording() {
	m_settings.hasData = false;
	sendData();
	serial.flush(true, true);
	// ofs.close();

}

void StimControl::setParameter(int param, float value) {
	if ( param == 0 ) {
		return;
	}
}

void StimControl::handleEvent(const EventChannel * eventInfo, const MidiMessage & message, int sampleNum) {
	if ( Event::getEventType(message) == EventChannel::TTL ) {
		TTLEventPtr ttl = TTLEvent::deserializeFromMessage(message, eventInfo);

		const int eventId = ttl->getState() ? 1 : 0;
		const int eventChannel = ttl->getChannel();

		const juce::int64 ts = ttl->getTimestamp(message);

		ofs << eventId << "\t" << ts << std::endl;

		if ( eventChannel == gateChannel ) {
			if ( eventId == 1 )
				state = true;
			else
				state = false;
			if ( state ) {
				if ( inputChannel == -1 || eventChannel == inputChannel ) {
					int64_t ts_std = static_cast<int64_t>(ts);
					if ( eventId == 0 ) {
						// low event received
						ofs << "0\t" << ts << std::endl;
						std::cout << "ts " << ts << std::endl;
					}
					else {
						// high signal received
						ofs << "1\t" << ts << std::endl;
						std::cout << "ts " << ts << std::endl;
					}
				}
			}
		}
	}
}

bool StimControl::enable() {
	acquisitionActive = true;
	return deviceSelected;
}

bool StimControl::disable() {
	acquisitionActive = false;
	return true;
}

AudioProcessorEditor * StimControl::createEditor() {
	editor = new StimControlEditor(this, true);
	return editor;
}

StimSettings StimControl::getSettings() {
	return m_settings;
}

void StimControl::sendData() {
	std::cout << "***********SENDING DATA**********\n";
	auto s = getSettings();
	printParams(s);
	serial.writeBytes((unsigned char *)&s, sizeof(StimSettings));
}

void StimControl::setPinStates() {
	auto editor = static_cast<StimControlEditor*>(getEditor());
	int inputPin = editor->getInputPin();
	int outputPin = editor->getOutputPin();
	m_settings.inputPin = static_cast<uint16_t>(inputPin);
	m_settings.outputPin = static_cast<uint16_t>(outputPin);
	m_settings.hasData = true;
}

void StimControl::setStartAndStopTimes() {
	auto editor = static_cast<StimControlEditor*>(getEditor());
	int startTime = editor->getStartTime();
	int stopTime = editor->getStopTime();
	m_settings.startTime = static_cast<uint16_t>(startTime);
	m_settings.stopTime = static_cast<uint16_t>(stopTime);
	m_settings.hasData = true;
}

void StimControl::setStimDurations() {
	auto editor = static_cast<StimControlEditor*>(getEditor());
	int stimOnDuration = editor->getStimOnTime();
	int stimOffDuration = editor->getStimOffTime();
	m_settings.stimOnTime = static_cast<uint16_t>(stimOnDuration);
	m_settings.stimOffTime = static_cast<uint16_t>(stimOffDuration);
	m_settings.hasData = true;
}

void StimControl::deviceInitialized(bool val) {
	isDeviceSetup = val;
}

bool StimControl::isDeviceInitialized() {
	return isDeviceSetup;
}
void StimControl::setupDevice() {
	auto editor = static_cast<StimControlEditor*>(getEditor());
	int selected = editor->deviceId;
	if ( selected >= 0 ) {
		std::cout << "Initializing device... " << std::endl;
		serial.setup(selected, baudrate);
		deviceInitialized(true);
		std::cout << "Device successfully initialized" << std::endl;
	}
	else
		CoreServices::sendStatusMessage("Select a device first.");
}

void StimControl::setupDevice(std::string devId) {
	serial.setup(devId, baudrate);
	deviceInitialized(true);
}

std::vector<ofSerialDeviceInfo> StimControl::getDeviceList(){
	return serial.getDeviceList();
}

void StimControl::closeDevice() {
	serial.close();
}

void StimControl::setDeviceString(std::string val) {
	devString = val;
}

std::string StimControl::getDeviceString() {
	return devString;
}

void StimControl::saveCustomParametersToXml(XmlElement* xml) {
	xml->setAttribute("Type", "StimControl");
	XmlElement * paramXml = xml->createNewChildElement("Parameters");
	auto current_settings = getSettings();
	paramXml->setAttribute("InputPin", static_cast<int>(current_settings.inputPin));
	paramXml->setAttribute("OutputPin", static_cast<int>(current_settings.outputPin));
	paramXml->setAttribute("StartTime", static_cast<int>(current_settings.startTime));
	paramXml->setAttribute("StopTime", static_cast<int>(current_settings.stopTime));
	paramXml->setAttribute("StimStartTime", static_cast<int>(current_settings.stimOnTime));
	paramXml->setAttribute("StimStopTime", static_cast<int>(current_settings.stimOffTime));

	XmlElement * deviceXml = xml->createNewChildElement("Devices");
	deviceXml->setAttribute("id", getDeviceString());
}

void StimControl::loadCustomParametersFromXml() {
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Parameters")
	{
		if ( paramXml->hasAttribute("InputPin") )
			m_settings.inputPin = static_cast<uint16_t>(paramXml->getIntAttribute("InputPin"));
		if ( paramXml->hasAttribute("OutputPin") )
			m_settings.outputPin = static_cast<uint16_t>(paramXml->getIntAttribute("OutputPin"));
		if ( paramXml->hasAttribute("StartTime") )
			m_settings.startTime = static_cast<uint16_t>(paramXml->getIntAttribute("StartTime"));
		if ( paramXml->hasAttribute("StopTime") )
			m_settings.stopTime = static_cast<uint16_t>(paramXml->getIntAttribute("StopTime"));
		if ( paramXml->hasAttribute("StimStartTime") )
			m_settings.stimOnTime = static_cast<uint16_t>(paramXml->getIntAttribute("StimStartTime"));
		if ( paramXml->hasAttribute("StimStopTime") )
			m_settings.stimOffTime = static_cast<uint16_t>(paramXml->getIntAttribute("StimStopTime"));
	}
	auto editor = static_cast<StimControlEditor*>(getEditor());
	editor->setStartTime(m_settings.startTime);
	editor->setStopTime(m_settings.stopTime);
	editor->setInputPin(m_settings.inputPin);
	editor->setOutputPin(m_settings.outputPin);
	editor->setStimOnTime(m_settings.stimOnTime);
	editor->setStimOffTime(m_settings.stimOffTime);
	forEachXmlChildElementWithTagName(*parametersAsXml, deviceXml, "Devices")
	{
		std::string device = deviceXml->getStringAttribute("id").toStdString();
		if ( ! device.empty() )
		{
			setupDevice(device);
			editor->setDeviceId(device);
			setDeviceString(device);
		}
	}
	setPinStates();
	setStartAndStopTimes();
	setStimDurations();
}

void StimControl::printParams(StimSettings settings) {
	std::cout << "Device is: " << devString << " with settings:\n"
	<< "Input pin: " << static_cast<int>(settings.inputPin) <<
	"\nOutput pin: " << static_cast<int>(settings.outputPin) <<
	"\nStart time (s): " << static_cast<int>(settings.startTime) <<
	"\nStop time (s): " << static_cast<int>(settings.stopTime) <<
	"\nStim on duration (ms): " << static_cast<int>(settings.stimOnTime) <<
	"\nStim off duration (ms): " << static_cast<int>(settings.stimOffTime) << std::endl;
}
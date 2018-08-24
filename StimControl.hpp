#ifndef STIMCONTROL_H_
#define STIMCONTROL_H_

#include <ProcessorHeaders.h>
#include <memory>
#include <SerialLib.h>

struct __attribute__ ((packed)) StimSettings {
	uint16_t inputPin = 0;
	uint16_t outputPin = 0;
	uint16_t startTime = 0;
	uint16_t stopTime = 0;
	uint16_t stimOnTime = 0;
	uint16_t stimOffTime = 0;
	bool hasData = false;
};

class StimControl : public GenericProcessor
{
private:
	bool state;
	bool acquisitionActive = false;
	bool deviceSelected;
	bool isDeviceSetup = false;
	int inputChannel;
	int outputChannel;
	int gateChannel;
	std::unique_ptr<juce::int64> initialTimeStamp = nullptr;
	unsigned int baudrate = 115200;
	ofSerial serial;
	StimSettings m_settings;
	std::string devString;
	void deviceInitialized(bool);
public:
	StimControl();
	~StimControl();
	bool isSource() const override { return false; }
	bool isSink() const override { return true; }
	bool hasEditor() const override { return true; }
	void startRecording() override;
	void stopRecording() override;
	void process(AudioSampleBuffer & buffer) override;
	void setParameter(int, float) override;
	void handleEvent(const EventChannel *, const MidiMessage &, int) override;
	bool enable() override;
	bool disable() override;
	AudioProcessorEditor * createEditor() override;
	std::vector<ofSerialDeviceInfo> getDeviceList();
	bool isDeviceInitialized();
	void setupDevice();
	void setupDevice(std::string);
	void setPinStates();
	void setStartAndStopTimes();
	void setStimDurations();
	void sendData();
	StimSettings getSettings();
	std::string getDeviceString();
	void setDeviceString(std::string);
	void closeDevice();
	void saveCustomParametersToXml(XmlElement *) override;
	void loadCustomParametersFromXml() override;

	void printParams(StimSettings);
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimControl);
};
#endif
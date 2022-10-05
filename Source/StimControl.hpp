#ifndef STIMCONTROL_H_
#define STIMCONTROL_H_

#include <ProcessorHeaders.h>
#include <memory>
#include <SerialLib.h>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

PACK(struct StimSettings {
	uint16_t inputPin = 0;
	uint16_t outputPin = 0; 
	uint16_t startTime = 0;
	uint16_t stopTime = 0;
	uint16_t stimOnTime = 0;
	uint16_t stimOffTime = 0;
	uint16_t hasData = 0;
});

auto const arduino_lines = Array<String>{"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"};

class StimControlSettings {
public:
	StimControlSettings() {};
	uint16_t inputPin = 0;
	uint16_t outputPin = 0;
	uint16_t startTime = 0;
	uint16_t stopTime = 0;
	uint16_t stimOnTime = 0;
	uint16_t stimOffTime = 0;
	uint16_t hasData = 0;
	std::string name = "";
	uint16_t deviceId = 0;
};

class StimControl : public GenericProcessor
{
private:
	bool state;
	bool acquisitionActive = false;
	bool deviceSelected = false;
	bool isDeviceSetup = false;
	int inputChannel = 13;
	int outputChannel = 13;
	int gateChannel = 13;
	std::unique_ptr<juce::int64> initialTimeStamp = nullptr;
	unsigned int baudrate = 9600;
	ofSerial serial;
	StimSettings m_settings;
	void deviceInitialized(bool);
	// std::string out_stream_file{"/home/robin/Dropbox/Science/Recordings/OpenEphys/testing/timestamps.txt"};
	std::ofstream ofs;
	StreamSettings<StimControlSettings> settings;
public:
	StimControl();
	~StimControl();
	void startRecording() override;
	void stopRecording() override;
	void process(AudioSampleBuffer & buffer) override;
	// void handleTTLEvent(TTLEventPtr) override;
	void parameterValueChanged(Parameter *param) override;
	void updateSettings() override;
	int sendStringToDevice(std::string const &);

	// void handleEvent(const EventChannel *, const MidiMessage &, int) override;
	AudioProcessorEditor * createEditor() override;
	std::vector<ofSerialDeviceInfo> getDeviceList();
	void getDeviceList(std::map<std::string, int>&);
	bool isDeviceInitialized();
	void setupDevice();
	void setupDevice(std::string);
	void sendData();
	StimSettings getSettings();
	std::string getDeviceString();
	// void setDeviceString(std::string);
	void closeDevice();
	void saveCustomParametersToXml(XmlElement *) override;
	void loadCustomParametersFromXml(XmlElement *) override;

	void printParams(StimSettings);
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimControl);
};
#endif
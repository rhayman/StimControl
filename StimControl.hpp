#ifndef STIMCONTROL_H_
#define STIMCONTROL_H_

#include <ProcessorHeaders.h>
#include <SerialLib.h>
#include <cstdint>
#include <memory>

#if defined(__GNUC__)
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define PACK(__Declaration__)                                                 \
  __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#else
#define PACK(__Declaration__) __Declaration__
#endif

// bool debug = true;

struct StimSettings {
  uint16_t inputPin = 0;
  uint16_t gatePin = 0;
  uint16_t outputPin = 0;
  uint16_t startTime = 0;
  uint16_t stopTime = 0;
  uint16_t stimOnTime = 0;
  uint16_t stimOffTime = 0;
  uint16_t hasData = 0;
};

PACK(struct StimBinaryPayload {
  uint8_t magic0 = 0xA5;
  uint8_t magic1 = 0x5A;
  uint8_t version = 1;
  uint8_t payloadSize = 0;
  uint16_t inputPin = 0;
  uint16_t gatePin = 0;
  uint16_t outputPin = 0;
  uint16_t startTime = 0;
  uint16_t stopTime = 0;
  uint16_t stimOnTime = 0;
  uint16_t stimOffTime = 0;
  uint16_t hasData = 0;
  uint16_t startRunning = 0;
});

auto const arduino_lines = Array<String>{"1", "2", "3",  "4",  "5",  "6", "7",
                                         "8", "9", "10", "11", "12", "13"};
auto const stim_protocols =
    Array<String>{"String commands", "Packed struct"};

class StimControlSettings {
public:
  StimControlSettings() {};
  uint16_t inputPin = 0;
  uint16_t gatePin = 2;
  uint16_t outputPin = 3;
  uint16_t protocol = 0;
  uint16_t startTime = 1;
  uint16_t stopTime = 2000;
  uint16_t stimOnTime = 5;
  uint16_t stimOffTime = 20;
  uint16_t hasData = 0;
  std::string name = "";
  uint16_t deviceId = 0;
};

class StimControl : public GenericProcessor {
private:
  bool isDeviceSetup = false;
  int inputChannel = 13;
  int outputChannel = 13;
  int gateChannel = 13;
  unsigned int baudrate = 9600;
  ofSerial serial;
  void deviceInitialized(bool);
  StreamSettings<StimControlSettings> settings;
  virtual void registerParameters() override;

public:
  StimControl();
  ~StimControl();
  bool startAcquisition() override;
  bool stopAcquisition() override;
  void startRecording() override;
  void stopRecording() override;
  void process(AudioBuffer<float> &buffer) override;
  void parameterValueChanged(Parameter *param) override;
  void updateSettings() override;
  int sendStringToDevice(std::string const &);
  int sendBinaryToDevice(StimBinaryPayload const &);
  uint16_t getUint16ParameterValue(const String &parameterName) const;
  void sanitizeSettings(StimSettings &) const;
  bool useBinaryProtocol() const;
  void sendData(bool startRunning);

  AudioProcessorEditor *createEditor() override;
  std::vector<ofSerialDeviceInfo> getDeviceList();
  void getDeviceList(std::map<std::string, int> &);
  bool isDeviceInitialized();
  void setupDevice();
  void setupDevice(std::string);
  StimSettings getSettings();
  std::string getDeviceString();
  void closeDevice();
  void printParams(StimSettings);

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimControl);
};
#endif

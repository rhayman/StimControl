#include "StimControl.hpp"
#include "../../../plugin-GUI/Source/Utils/Utils.h"
#include "StimControlEditor.hpp"

#include <stdio.h>

StimControl::StimControl()
    : GenericProcessor("StimControl"), outputChannel(13), inputChannel(-1),
      state(true), acquisitionActive(false), deviceSelected(false) {}

void StimControl::registerParameters() {
  std::map<std::string, int> devices;
  getDeviceList(devices);
  Array<String> devs;
  for (auto dev : devices)
    devs.add(dev.first);
  addCategoricalParameter(Parameter::PROCESSOR_SCOPE, "device", "Device name",
                          "Devices available", devs, 0);
  addTtlLineParameter(Parameter::STREAM_SCOPE, "Trigger", "Trigger line",
                      "Trigger line on Arduino", arduino_lines.size(), 1);
  addTtlLineParameter(Parameter::STREAM_SCOPE, "Gate", "Gate line",
                      "Gate line on Arduino", arduino_lines.size(), 1);
  addTtlLineParameter(Parameter::STREAM_SCOPE, "Output", "Output line",
                      "Output line on Arduino", arduino_lines.size(), 1);
  addStringParameter(Parameter::PROCESSOR_SCOPE, "Start", "Start time (s)",
                     "Start time", "0");
  addStringParameter(Parameter::PROCESSOR_SCOPE, "Stop", "Stop time (s)",
                     "Stop time", "2000");
  addStringParameter(Parameter::PROCESSOR_SCOPE, "Duration",
                     "Stimulation duration (ms)", "Stim duration", "10");
  addStringParameter(Parameter::PROCESSOR_SCOPE, "Interval",
                     "Interval duration(ms)", "interval duration", "150");
  addBooleanParameter(Parameter::PROCESSOR_SCOPE, "Apply", "",
                      "apply during recording", true);
  // parameterValueChanged(getParameter("Device"));
}

StimControl::~StimControl() {
  sendStringToDevice("<StartRunning, 0, >");
  serial.flush(true, true);
  serial.close();
}

void StimControl::process(AudioBuffer<float> &buffer) {
  LOGD("StimControl - in process");
}

bool StimControl::startAcquisition() {
  if (!isDeviceInitialized()) {
    setupDevice();
  }
  sendData();
  serial.flush(true, true);
  LOGD("StimControl - Acquisition started");
  return true;
}

bool StimControl::stopAcquisition() {
  sendStringToDevice("<StartRunning, 0, >");
  serial.flush(true, true);
  return true;
}

void StimControl::startRecording() {
  if (!isDeviceInitialized()) {
    setupDevice();
  }
  sendData();
  LOGD("StimControl - Recording started");
  serial.flush(true, true);
}

void StimControl::stopRecording() {
  sendStringToDevice("<StartRunning, 0, >");
  serial.flush(true, true);
}

void StimControl::parameterValueChanged(Parameter *param) {
  for (auto stream : getDataStreams()) {
    if (stream->getName().equalsIgnoreCase("StimControl datastream")) {
      auto this_dev = settings[stream->getStreamId()];
      if (param->getName().equalsIgnoreCase("device")) {
        LOGD("StimControl - device parameter changed: ", param->getName());
        this_dev->name = ((CategoricalParameter *)(param))
                             ->getSelectedString()
                             .toStdString();
        std::map<std::string, int> devices;
        getDeviceList(devices);
        this_dev->deviceId = devices[this_dev->name];
        setupDevice();
      } else if (param->getName().equalsIgnoreCase("Trigger")) {
        this_dev->inputPin = (int)param->getValue();
        //   this_dev->inputPin = ((CategoricalParameter *)(param))
        //                            ->getSelectedString()
        //                            .getIntValue();
      } else if (param->getName().equalsIgnoreCase("Gate")) {
        this_dev->gatePin = (int)param->getValue();
        //   // this_dev->name =
        //   //
        //   ((CategoricalParameter*)(param))->getSelectedString().toStdString();
      } else if (param->getName().equalsIgnoreCase("Output")) {
        this_dev->outputPin = (int)param->getValue();
        //   this_dev->outputPin = ((CategoricalParameter *)(param))
        //                             ->getSelectedString()
        //                             .getIntValue();
      } else if (param->getName().equalsIgnoreCase("Start")) {
        this_dev->startTime = (int)param->getValue();
      } else if (param->getName().equalsIgnoreCase("Stop")) {
        this_dev->stopTime = (int)param->getValue();
      } else if (param->getName().equalsIgnoreCase("Duration")) {
        this_dev->stimOnTime = (int)param->getValue();
      } else if (param->getName().equalsIgnoreCase("Interval")) {
        this_dev->stimOffTime = (int)param->getValue();
      }
    }
  }
}

void StimControl::updateSettings() {

  if (getDataStreams().isEmpty()) {
    DataStream::Settings streamsettings{
        "StimControl datastream", "Datastream for stimulation from Arduino",
        "external.stimulation.arduino", getDefaultSampleRate()};

    auto stream = new DataStream(streamsettings);
    dataStreams.add(stream);
    dataStreams.getLast()->addProcessor(this);

    ContinuousChannel::Settings settings{
        ContinuousChannel::Type::AUX, "DUMMY", "stim channel", "stim.raw", 1,
        dataStreams.getLast()};
    continuousChannels.add(new ContinuousChannel(settings));
  }
  settings.update(getDataStreams());
  parameterValueChanged(getParameter("device"));
  isEnabled = true;
  LOGD("StimControl - Settings updated");
}

AudioProcessorEditor *StimControl::createEditor() {
  editor = std::make_unique<StimControlEditor>(this);
  return editor.get();
}

StimSettings StimControl::getSettings() {
  StimSettings current_settings;
  for (auto stream : getDataStreams()) {
    if (stream->getName().equalsIgnoreCase("StimControl datastream")) {
      auto this_dev = settings[stream->getStreamId()];
      current_settings.hasData = 1;
      current_settings.inputPin = this_dev->inputPin;
      current_settings.outputPin = this_dev->outputPin;
      current_settings.startTime = (int)getParameter("Start")->getValue();
      current_settings.stopTime = (int)getParameter("Stop")->getValue();
      current_settings.stimOffTime = (int)getParameter("Interval")->getValue();
      current_settings.stimOnTime = (int)getParameter("Duration")->getValue();
    }
  }
  LOGD("StimControl - getting Settings");
  return current_settings;
}

void StimControl::sendData() {
  if (!isDeviceInitialized())
    setupDevice();
  CoreServices::sendStatusMessage("Sending data");
  StimSettings s = getSettings();
  printParams(s);
  LOGD("StimControl - sending data to device");
  int bytes_sent =
      sendStringToDevice("<Start," + std::to_string(s.startTime) + ",>");
  //   bool debug = false;
  // #if defined(DEBUG)
  //   debug = true;
  // #endif
  //   if (debug)
  //     LOGD("Bytes sent for startTime: ", bytes_sent);
  //   bytes_sent = sendStringToDevice("<Stop," + std::to_string(s.stopTime) +
  //   ",>"); if (debug)
  //     LOGD("Bytes sent for stopTime: ", bytes_sent);
  //   bytes_sent =
  //       sendStringToDevice("<OutputPin," + std::to_string(s.outputPin) +
  //       ",>");
  //   if (debug)
  //     LOGD("Bytes sent for outputPin: ", bytes_sent);
  //   bytes_sent =
  //       sendStringToDevice("<Duration," + std::to_string(s.stimOnTime) +
  //       ",>");
  //   if (debug)
  //     LOGD("Bytes sent for duration: ", bytes_sent);
  //   bytes_sent =
  //       sendStringToDevice("<Interval," + std::to_string(s.stimOffTime) +
  //       ",>");
  //   if (debug)
  //     LOGD("Bytes sent for interval: ", bytes_sent);
  auto startRunning = (bool)getParameter("Apply")->getValue();
  bytes_sent = sendStringToDevice("<StartRunning," +
                                  std::to_string(startRunning) + ",>");
  // if (debug)
  //   LOGD("Bytes sent for startRunning: ", bytes_sent);
  LOGD("StimControl - data sent to device");
  CoreServices::sendStatusMessage("Data sent");
}

int StimControl::sendStringToDevice(std::string const &str) {
  std::vector<unsigned char> buffer(str.begin(), str.end());
  LOGD("StimControl - sending string to device: ", str);
  auto res = serial.writeBytes(buffer.data(), buffer.size());
  LOGD("StimControl - sent string to device: ", str, " (", res, " bytes)");
  return res;
}

void StimControl::deviceInitialized(bool val) { isDeviceSetup = val; }

bool StimControl::isDeviceInitialized() { return isDeviceSetup; }
void StimControl::setupDevice() {
  for (auto stream : getDataStreams()) {
    if (stream->getName().equalsIgnoreCase("StimControl datastream")) {
      auto this_dev = settings[stream->getStreamId()];
      auto selected = this_dev->deviceId;
      if (selected >= 0) {
        CoreServices::sendStatusMessage("Initializing device...");
        bool success = serial.setup(selected, baudrate);
        deviceInitialized(success);
        if (success)
          CoreServices::sendStatusMessage("Device successfully initialized");
        else
          CoreServices::sendStatusMessage("Device not initialized");
      } else
        CoreServices::sendStatusMessage("Select a device first.");
    }
  }
}

void StimControl::setupDevice(std::string devId) {
  bool success = serial.setup(devId, baudrate);
  deviceInitialized(success);
  if (success)
    CoreServices::sendStatusMessage("Device successfully initialized");
  else
    CoreServices::sendStatusMessage("Device not initialized");
}

std::vector<ofSerialDeviceInfo> StimControl::getDeviceList() {
  return serial.getDeviceList();
}

void StimControl::getDeviceList(std::map<std::string, int> &namesAndIDs) {
  auto devices = getDeviceList();
  for (auto dev : devices) {
    namesAndIDs[dev.getDeviceName()] = dev.getDeviceID();
  }
}

void StimControl::closeDevice() { serial.close(); }

void StimControl::printParams(StimSettings settings) {
  LOGD("Has data: ", settings.hasData);
  LOGD("Input pin: ", settings.inputPin);
  LOGD("Output pin: ", settings.outputPin);
  LOGD("Start time (s): ", settings.startTime);
  LOGD("Stop time (s): ", settings.stopTime);
  LOGD("Stim on duration (ms): ", settings.stimOnTime);
  LOGD("Stim off duration (ms): ", settings.stimOffTime);
}

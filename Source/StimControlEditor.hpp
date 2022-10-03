#ifndef STIMCONTROLEDITOR_H_
#define STIMCONTROLEDITOR_H_

#include <EditorHeaders.h>
#include "StimControl.hpp"

class StimControlEditor : public GenericEditor
{
public:
	StimControlEditor(GenericProcessor *);
	~StimControlEditor() {};
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimControlEditor);
};

#endif
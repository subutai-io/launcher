#include "LibraryItemTab.h"
#include "LibraryComponent.h"

LibraryItemTab::LibraryItemTab(const std::string& title, int order, bool highlight) :
    _title(title),
    _order(order),
    _selected(highlight)
{
    auto font = Font(16);
    _titleLabel.setText(_title, dontSendNotification);
    if (_selected) 
    {
        _titleLabel.setColour(Label::textColourId, Colours::white);
    } 
    else 
    {
        _titleLabel.setColour(Label::textColourId, Colours::grey);
    }
    _titleLabel.setBounds(0, 0, _title.length() * 12, 30);
    _titleLabel.setFont(font);
    _titleLabel.setJustificationType(Justification::centred);
    _titleLabel.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(_titleLabel);
    setSize(_title.length() * 12, 30);
}

LibraryItemTab::~LibraryItemTab()
{

}

void LibraryItemTab::paint(juce::Graphics& g)
{
    _titleLabel.setColour(Label::textColourId, Colours::grey);
    if (_selected) {
	    g.setColour(Colour(7, 141, 208));
        g.drawLine(0, 28, getWidth(), 28, 3);
        _titleLabel.setColour(Label::textColourId, Colours::white);
    } else if (_hover) {
	    g.setColour(Colour(200, 200, 200));
        g.drawLine(0, 28, getWidth(), 28, 3);
    }
}

void LibraryItemTab::resized()
{

}

std::string LibraryItemTab::title()
{
    return _title;
}

void LibraryItemTab::mouseEnter(const juce::MouseEvent& e)
{
    _hover = true;
    repaint();
}

void LibraryItemTab::mouseExit(const juce::MouseEvent& e)
{
    _hover = false;
    repaint();
}

void LibraryItemTab::mouseUp(const juce::MouseEvent& e)
{
    auto p = (LibraryComponent*)getParentComponent();
        p->tabClick(_title);
}

void LibraryItemTab::select()
{
    _hover = false;
    _selected = true;
    repaint();
}

void LibraryItemTab::deselect()
{
    _selected = false;
    repaint();
}

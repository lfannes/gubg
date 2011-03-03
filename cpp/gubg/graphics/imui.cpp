#include "graphics/imui.hpp"
#include "timer.hpp"
#include "sleep.hpp"
#include "bitmagic.hpp"
using namespace gubg;

bool IMUI::processInput()
{
    if (processInput_())
        somethingChanged_ = true;
    return somethingChanged_;
}

void IMUI::reset()
{
    somethingChanged_ = false;
}

bool IMUI::waitForInput(double timeout)
{
    Timer timer(ResetType::NoAuto);
    while (timer.difference() < timeout)
    {
        if (processInput())
            return true;
        //10ms resolution should be fast enough for most things
        nanosleep(0, 10000000);
    }
    return false;
}

Key IMUI::getLastKey()
{
    Key key = lastKey_;
    lastKey_ = Key::Nil;
    return key;
}

bool IMUI::getDigit(unsigned char &digit)
{
    if (convertToDigit(digit, lastKey_))
    {
        getLastKey();
        return true;
    }
    return false;
}

bool IMUI::escapeIsPressed()
{
    if (Key::Escape == lastKey_)
    {
        getLastKey();
        return true;
    }
    return false;
}

bool IMUI::isMouseInside(const TwoPoint<> &region) const
{
    return region.isInside(mousePosition_);
}
bool IMUI::checkMouseButton(MouseButton button, ButtonState cmpState)
{
    bool isUp = false, changed = false;
    switch (button)
    {
        case MouseButton::Left: leftMouse_.get(isUp, changed); break;
        case MouseButton::Middle: middleMouse_.get(isUp, changed); break;
        case MouseButton::Right: rightMouse_.get(isUp, changed); break;
        case MouseButton::WheelUp: wheelUp_.get(isUp, changed); break;
        case MouseButton::WheelDown: wheelDown_.get(isUp, changed); break;
    }
    switch (cmpState)
    {
        case ButtonState::IsDown: return false == isUp; break;
        case ButtonState::IsUp: return true == isUp; break;
        case ButtonState::IsOrWentDown: return false == isUp || changed; break;
        case ButtonState::IsOrWentUp: return true == isUp || changed; break;
    }
    //Should never be reached
    assert(false);
    return false;
}

void Widgets::WidgetProxy::set(std::auto_ptr<IWidget> widget)
{
    //Internally, we keep the object in a shared_ptr since WidgetProxy has to be storable in an STL container
    widget_.reset(widget.release());
}

WidgetState Widgets::WidgetProxy::process()
{
    if (!widget_)
        return WidgetState::Empty;
    return widget_->process();
}

Widgets::WidgetProxy &Widgets::get(unsigned int extra)
{
    void *callerLocation;
#if 0
    asm
    {
        mov EAX, dword ptr [EBP+4];
        mov callerLocation[EBP], EAX;
    }
#endif
    unsigned int id = createId_(callerLocation, extra);
    return widgetPerId_[id];
}
unsigned int Widgets::createId_(void *location, unsigned int extra)
{
    return (unsigned int)location ^ bitmagic::reverseBits(extra);
}

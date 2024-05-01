#ifndef MYSTIMULUS_H
#define MYSTIMULUS_H

#include <map>
#include <set>

class MyStimulus
{
public:
    MyStimulus() : mTag(0)
    {
    }
    virtual ~MyStimulus()
    {
    }
    // Properties
    MyStimulus& SetTag(int inTag)
    {
        mTag = inTag;
        return *this;
    }
    int Tag() const
    {
        return mTag;
    }
    MyStimulus& SetLabel(std::string sLable)
    {
        mLable = sLable;
        return *this;
    }
    std::string Lable() const
    {
        return mLable;
    }
    // Event calling interface
    MyStimulus& Present()
    {
        OnPresent();
        return *this;
    }
    MyStimulus& Conceal()
    {
        OnConceal();
        return *this;
    }
    MyStimulus& Repeate() {
        OnRepeate();
        return *this;
    }
    bool isPlaying() {
        return OnIsPlaying();
    }
protected:
    // Event handling interface
    //  In its OnPresent event handler, a stimulus is supposed to present itself
    //  (e.g., to make itself visible, play itself if it is a sound or a movie,
    //  or highlight itself if it is a P300 matrix element).
    virtual void OnPresent() = 0;
    //  In its OnConceal event handler, a stimulus is supposed to conceal itself
    //  (e.g., make itself invisible, stop playback, or switch back to normal mode).
    //  This event is called Conceal rather than Hide because "Hide" is already
    //  used for making a graphic element invisible.
    virtual void OnConceal() = 0;
    virtual void OnRepeate() = 0;
    virtual bool OnIsPlaying() = 0;

private:
    int mTag;
    std::string mLable;
};

class SetOfMyStimuli : public std::set<MyStimulus*>
{
public:
    SetOfMyStimuli()
    {
    }
    virtual ~SetOfMyStimuli()
    {
    }
    // Householding
    SetOfMyStimuli& Add(MyStimulus* s)
    {
        insert(s);
        return *this;
    }
    SetOfMyStimuli& Remove(MyStimulus* s)
    {
        erase(s);
        return *this;
    }
    SetOfMyStimuli& Clear()
    {
        clear();
        return *this;
    }
    SetOfMyStimuli& DeleteObjects()
    {
        for (iterator i = begin(); i != end(); ++i)
            delete* i;
        clear();
        return *this;
    }

    bool Contains(MyStimulus* s) const
    {
        return find(s) != end();
    }
    bool Intersects(const SetOfMyStimuli& s) const
    {
        for (const_iterator i = begin(); i != end(); ++i)
            if (s.Contains(*i))
                return true;
        return false;
    }

    // Events
    void Present()
    {
        for (iterator i = begin(); i != end(); ++i)
            (*i)->Present();
    }
    void Conceal()
    {
        for (iterator i = begin(); i != end(); ++i)
            (*i)->Conceal();
    }
    void repeate()
    {
        for (iterator i = begin(); i != end(); ++i)
            (*i)->Repeate();
    }
};

#endif // MYSTIMULUS_H

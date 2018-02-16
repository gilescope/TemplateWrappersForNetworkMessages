#include <cstdlib>
#include <memory>

#include "Listing1.h"


template <class T> class MsgPart {
public:
    //      Type definitions for convenience:
    typedef T Type;
    typedef MsgBuf::Size Size;
    typedef const T &ConstRef;
    //      Calculated current (actual from the message)
    //      and default size:
    static inline Size size (const MsgBuf &buffer, Size bufOffset)
    {return sizeof (T);}
    static inline Size defaultSize ()
    {return sizeof (T);}
    //      Assign to the object's buffer representation:
    static inline void
    assign (MsgBuf &b, Size bufOffset, ConstRef value)
    {* (T*) (b.buffer (bufOffset + sizeof (T)) + bufOffset) =
             value;}
    //      initialize the object:
    static inline void initialize (MsgBuf &b, Size bufOffset)
    {memset (b.buffer (bufOffset + sizeof (T)) + bufOffset,
             0, sizeof (T));}
};

template <> class MsgPart <char*> {
public:
    //      Type definitions for convenience:
    typedef char *Type;
    typedef MsgBuf::Size Size;
    typedef const char *ConstRef;
    //      Calculated (actual) or default size:
    static inline Size size (const MsgBuf &buf, Size bufOffset)
    {
        const char *b = buf + bufOffset;
        const void *p = memchr (b, 0, buf.length () - bufOffset);
        return p ? ((const char*) p - b + 1) : 0;
    }
    static inline Size defaultSize ()
    {
        return sizeof (char);
    }
    //      Assign to the object's buffer representation:
    static inline void
    assign (MsgBuf &b, Size bufOffset, ConstRef value)
    {
        Size s = value ? strlen (value) : 0;
        b.resizePart (bufOffset, size (b, bufOffset), s + 1);
        if (value) memcpy (b.buffer (bufOffset + s) + bufOffset,
                           value, s);
        b.buffer (bufOffset + s + 1) [bufOffset + s] = 0;
    }
    //      Initialize the object:
    static inline void initialize (MsgBuf &b, Size bufOffset)
    {
        b.buffer (bufOffset + 1) [bufOffset] = 0;
    }
};

template <class T> class MsgPartRef {
public:
    //      Convenience types:
    typedef MsgPart <T> Definition;
    typedef MsgBuf::Size Size;
private:
    //      The containing message and offset:
    MsgBuf &buffer;
    Size offset;
    //      Prevent accidental miss-use:
    MsgPartRef <T> &operator = (const MsgPartRef <T> &);
public:
    //      Ctors:
    inline MsgPartRef (MsgBuf &b, Size o)
            : buffer (b), offset (o) {}
    inline MsgPartRef (const MsgPartRef <T> &o)
            : buffer (o.buffer), offset (o.offset) {}
    //      Assignment:
    template <class A> inline MsgPartRef &operator = (A v)
    {Definition::assign (buffer, offset, v); return *this;}
    inline MsgPartRef &operator = (typename Definition::ConstRef v)
    {Definition::assign (buffer, offset, v); return *this;}
    //      Allow read-only use as an object of the required type:
    inline operator typename Definition::ConstRef () const
    {return * (T*) ((const char*) buffer + offset);}
};

class Message {
public:
    //      Convenience type:
    typedef MsgBuf::Size Size;
protected:
    //      The message buffer:
    MsgBuf buffer;
    //      Access to portions of the message,
    //      and related information:
    inline char* value (Size pos, Size s)
    {return buffer.buffer (pos + s) + pos;}
    inline const char *value (Size pos) const
    {return (const char*) buffer + pos;}
    //      The offset of this portion of the message
    //      into the buffer:
    inline Size offset () const
    {return 0;}
    //      The size of this part of the message:
    inline Size partSize (Size) const
    {return 0;}
    //      The total size of this and all previous parts
    //      of the message:
    inline Size totalSize () const
    {return 0;}
public:
    //      Ctor, taking initial size if user expects growth
    //      or if derived class knows its size (for efficiency):
    inline Message (Size initContentSize = 0)
            : buffer (initContentSize) {}
    //      Copy ctor (note message buffer is
    //      delayed-copy on write):
    inline Message (const Message &o)
            : buffer (o.buffer) {}
    //      Ctor taking a message buffer (for re-construction):
    inline Message (const MsgBuf &b)
            : buffer (b) {}
    //      Dtor not required to be virtual:
    //      derived only via MsgExtension, which has no members
    //      Get the raw message:
    Size size () const
    {return buffer.length ();}
    const char *rawData () const
    {return buffer;}
};

template <class B, class T> class MsgExtension : public B {  //Was B...did he mean that?
public:
    //      Convenience types:
    typedef MsgBuf::Size Size;
    typedef T Part;
    typedef B Base;
    typedef MsgPart <T> Definition;
private:
    //      Inline methods just to ensure parameter is
    //      calculated only once:
    inline Size calcOffset (Size p) const
    {return p + B::partSize (p);}
    inline Size calcSize (Size p) const
    {return p + partSize (p);}
protected:
    //      Calculate the offset in the message buffer
    //      of this part:
    inline Size offset () const
    {return calcOffset (B::offset ());}
    //      The size of this part:
    inline Size partSize (Size p) const
    {return Definition::size (B::buffer, p);}
    //      Total size of this and all previous parts:
    inline Size totalSize () const
    {return calcSize (B::totalSize ());}
public:
    //      Ctor taking initial size of 'rest' of message
    //      (ie, excluding this and preceding content):
    inline MsgExtension (Size initContentSize = 0)
            : B (initContentSize + Definition::defaultSize ())
    {Definition::initialize (B::buffer, offset ());}
    //      Use default assignment operator and copy ctor
    //      Ctor overlaying another message:
    inline MsgExtension (const Message &m)
            : B (m)
    {
        //      Check the overlaid message is large enough:
        if (B::buffer.length () < B::size ())
            throw "Overlaid message too small";
    }
    //      Access to the message data:
    inline MsgPartRef <T> part ()
    {return MsgPartRef <T> (B::buffer, offset ());}
    typename Definition::ConstRef part () const
    {return * (const Part*) value (offset ());}
    //      Wrappers to 'part()', use by
    //      'msg.MessageStruct::operator->()->field',
    //      or by the neater 'msg.MessageStruct::data()->field'.
    //      (data() canít return pointer to part, as this would
    //      allow direct access to variable-size parts).
    //      For 'whole' part, access by
    //      "msg.MessageStruct::part ()" These should only be
    //      instantiated when used, ie, for struct T
    inline MsgExtension <B, T> &data () {return *this;}
    inline const MsgExtension <B, T> &data () const {return *this;}
    inline Part *operator -> ()
    {
        Size o = offset ();
        return (Part*) B::value (o, partSize (o));
    }
    inline const Part *operator -> () const
    {return (const Part*) value (offset ());}
};
/* End of File */


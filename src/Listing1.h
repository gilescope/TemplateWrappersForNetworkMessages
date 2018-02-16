#include <cstdlib>
#include <memory>
#include <string>

template <class _Char, class _Allocator = std::allocator <_Char> >
class BasicMsgBuf {
private:
    typedef
    std::basic_string<_Char, std::char_traits <_Char>, _Allocator>
            Content;
    Content content;
public:
    typedef typename Content::size_type Size;
    //      Ctors: giving initial size or a raw data blob:
    BasicMsgBuf (Size initSize = 0)
            : content (initSize, '\0') {}
    BasicMsgBuf (const BasicMsgBuf <_Char, _Allocator> &o)
            : content (o.content) {}
    //      Ctor taking initial content
    //      (for messages re-construction)
    BasicMsgBuf (const char *d, Size s)
            : content (d, s) {}
    //      Default dtor and assignment.
    //      Read-only access:
    operator const char* () const
    {return content.c_str ();}
    Size length () const
    {return content.length ();}
    //      Read-write access, with minimum buffer size specified:
    char *buffer (Size l)
    {
        content.reserve (l);
        return &(content [0]);
    }
    //      Resize a portion of the buffer, shuffling the remainder
    //      if necessary:
    void resizePart (Size pos, Size oldSize, Size newSize)
    {
        if (oldSize < newSize) {
            content.insert (pos, newSize - oldSize, '\0');
        } else if (oldSize > newSize) {
            content.erase (pos, oldSize - newSize);
        }
    }
};

typedef BasicMsgBuf <char> MsgBuf;
/* End of File */
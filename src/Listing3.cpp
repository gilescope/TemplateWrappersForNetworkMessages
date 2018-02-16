

#include <string>
#include <iostream>

using namespace std;

//#include "Listing1.h"
#include "Listing2.h"


struct HeaderData {
    char source [4];
    unsigned int  typeId;
};

//      Build up the message one part at a time:
typedef MsgExtension <Message, HeaderData> Header;
typedef MsgExtension <Header, char*> Message4Text;
typedef MsgExtension <Message4Text, int> Message4;

int main (int argc, const char **argv)
{
    for (int i; i < 10000000; ++i) {
        Message4 a;
        //      Use .baseclass:: to access the messageís component
        //      parts (base classes):
        memcpy(a.Header::data()->source, "Fred", 4);
        a.Header::data()->typeId = 4;
        a.Message4Text::part() = "This is a variable length string";
        //      'Send' this message:
        const char *data = a.rawData();
        int size = a.size();
        //      'Receive' the message:
        MsgBuf receivedData;
        memcpy(receivedData.buffer(size), data, size);
        Header receivedMsg(receivedData);
        //      Translate the received message to the correct type:

        if (receivedMsg->typeId == 4) {
            Message4 b(receivedMsg);
            //      Now process the message:
            //      Use down-casting to get at the Header
            //      base-class of b:
            Header &header = (Header &) b;
            header->typeId = 16;
            memcpy(header->source, "Jane", 4);
            //      Use the content of the received msg:
            b.Message4::part() = 128;
        }
    }
    cout << "Hello" <<endl;
    return 0;
}
//End of File


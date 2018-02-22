@0xdb19af1f14bf0b36;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("message");

struct Message4 {
    header @0 :Header;
    message @1 :Text;
    anInt @2 :UInt32;

    struct Header {
        source @0 :Text;
        typeId @1 :UInt8;
    }
}

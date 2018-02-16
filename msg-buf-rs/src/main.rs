#[macro_use] extern crate serde_derive;
extern crate serde;

extern crate rmp_serde as rmps;
extern crate rmpv;

use serde::{Deserialize, Serialize};
use rmps::{Deserializer, Serializer};

#[derive(Serialize, Deserialize, Debug)]
struct Header {
    source: [char; 4],
    type_id: usize,
}

#[derive(Serialize, Deserialize, Debug)]
struct Message4 {
    header: Header,
    message4_text: String,
    an_int: i32,
}

pub fn main()
{
    for _i in 0..10_000_000 {
        let a = Message4{
            header: Header{ source: ['F','r','e','d'], type_id: 4 },
            message4_text: String::from("This is a variable length string"),
            an_int: 126
        };

        // 'Send' this message:
        let mut data = Vec::new();
        a.serialize(&mut Serializer::new(&mut data)).unwrap();
        let _size: usize = data.len();
        //      'Receive' the message:
        let mut de = Deserializer::new(&data[..]);
        let mut received_msg: Message4 = Deserialize::deserialize(&mut de).unwrap();

        // Translate the received message to the correct type:
        if received_msg.header.type_id == 4 {
            received_msg.header.type_id = 16;
            received_msg.header.source = ['J', 'a', 'n', 'e'];
            // Use the content of the received msg:
            received_msg.an_int = 128;
        }
    }
    println!("Hello");
    std::process::exit(0);
}